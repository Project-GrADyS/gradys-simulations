#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "TwoRayNewModel.h"
#include <complex>



namespace gradys_simulations {

Define_Module(TwoRayNewModel);

// shortcut for squaring numbers (should be faster than pow(x, 2.0))
namespace { constexpr double squared(double x) { return x * x; } }


/**
 * Literature referenced here:
 * Rappaport, Theodore: Wireless Communications - Principles and Practice, 2nd edition, 2002
 * Jakes, William C.: "Microwave Mobile Communications", 1974
 * C�digos e Slides Matlab Maj Andrezo
 */
/** Foram feitas mudan�as no calculo do coef de reflex�o (Gamma)
 * epsilon relativo foi setado como 15 (permissividade do chao/permissividade do ar)
 * permissividade do ar = 1
 * permissividade do ch�o = 15; media
 * inclus�o da condutividade do ch�o no c�lculo
 * Codigo editado para a inclus�o da condutividade, com o coeficiente de reflex�o se tornando complexo
 * valores tirados da simula��o do Major Andrezo
 * Inclusa condi��o de terra plana e terra esf�rica (antes era apenas terra plana)
 */

TwoRayNewModel::TwoRayNewModel() :
    epsilon_r(1.0), polarization('h'), sigma(1.0), delta_h(1.0)
{
}

void TwoRayNewModel::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        epsilon_r = par("epsilon_r");
        sigma = par("sigma");
        delta_h = par("delta_h"); //Ground height variations
        const std::string polarization_str = par("polarization");
        if (polarization_str == "horizontal") {
            polarization = 'h';
        } else if (polarization_str == "vertical") {
            polarization = 'v';
        } else {
            throw cRuntimeError("Invalid antenna polarization %s", polarization_str.c_str());
        }
    }
}

std::ostream& TwoRayNewModel::printToStream(std::ostream& os, int level, int evFlags = 0) const
{
    os << "TwoRayNewModel";
    if (level >= PRINT_LEVEL_TRACE)
        os << ", epsilon_r = " << epsilon_r << ", polarization = " << polarization<< ",sigma ="<< sigma<<",delta_h = "<< delta_h;
    return os;
}

double TwoRayNewModel::computePathLoss(const ITransmission* transmission, const IArrival* arrival) const
{
    auto radioMedium = transmission->getMedium();
    auto narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    const mps propagationSpeed = radioMedium->getPropagation()->getPropagationSpeed();
    const Hz centerFrequency = Hz(narrowbandSignalAnalogModel->getCenterFrequency());
    const m waveLength = propagationSpeed / centerFrequency;
    return computeTwoRayNewModel(transmission->getStartPosition(), arrival->getStartPosition(), waveLength);
}

double TwoRayNewModel::computeTwoRayNewModel(const Coord& pos_t, const Coord& pos_r, m lambda) const

//Fun��o para decidir se o modelo a ser executado ser� o de terra plana ou terra esf�rica

{
    double d_los = pos_r.distance(pos_t);
    const double Req = 1.33*6371000; //raio equivalente da terra
    if(pos_t.z>=pos_r.z){    //testa qual dos hosts est� a maior altura
        const double d_h =sqrt(2*Req*pos_t.z);  //distancia horizonte radio
        if(d_los<=d_h){     //condi��o de terra plana, calculos como vistos aqui
            return terraPlana(pos_t, pos_r, lambda);

                }   else{
           //terra esf�rica
            return terraEsferica(pos_t, pos_r, lambda);
        }
    }   else {

        const double d_h = sqrt(2*Req*pos_r.z);
        if(d_los<=d_h){     //condi��o de terra plana, calculos como vistos aqui
            return terraPlana(pos_t, pos_r, lambda);

         }  else{
          //condi��o de terra esf�rica
            return terraEsferica(pos_t, pos_r, lambda);
        }
    }
}

double TwoRayNewModel::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    return NaN;
}

m TwoRayNewModel::computeRange(mps propagationSpeed, Hz frequency, double loss) const
{
    return m(NaN);
}


double TwoRayNewModel::terraEsferica(const Coord& pos_t, const Coord& pos_r, m lambda) const
{
    /*
     * Parte geom�trica, baseada nos c�digos em Matlab do Major Andrezo e nas equa��es de Norton
     *
     */
    double beta = 2*M_PI/(lambda.get()); //constante de fase
    double Req = 1.33*6371000;

    double d = pos_r.distance(pos_t);  //Distancia entre as antenas (n�o sei se est� certo)
    double h_t = pos_t.z; //altura do transmissor em rela��o ao solo
    double h_r = pos_r.z;//altura do receptor em rela��o ao solo
    //Calculando distancia para o ponto de reflex�o no solo

    double p = 2*sqrt((Req*(h_t+h_r)+(d*d)/4)/3);
    double phi = acos(2*Req*abs(h_t-h_r)*d/(pow(p,3)));
    double d1 = d/(2+p*cos((phi+M_PI)/3)); //distancia para o ponto de reflex�o na superficie para tx
    double ht = h_t - (pow(d1,2)/2*Req); //altura equivalente de tx

    //resolver raiz quadrada
    double delta = pow((2*Req),2) - 4*(pow((d1/ht),2))*(-2*Req*h_r);
    double raiz1 = (-2*Req + sqrt(delta))/(2*pow((d1/ht),2));
    double raiz2 = (-2*Req - sqrt(delta))/(2*pow((d1/ht),2));
    double raiz;
    if(raiz1 >0){
        raiz = raiz1;
        }
    else if(raiz2 >2){

        raiz = raiz2;
    }
    //considera como resultado a raiz positiva
    double hr = raiz; //altura equivalente de rx
    double d2 = (hr*d1)/ht;

    double r_d = sqrt(pow(d,2)+pow(ht-hr,2)); //trajeto do raio direto
    double r_r = sqrt(pow(d,2)+pow(ht+hr,2)); //trajeto do raio refletido
    double dr  = abs(r_r - r_d);    //diferen�a entre os raios

    //fator de divergencia
    double D = pow(1+(2*d1*d2)/(Req*(ht+hr)),(-1/2));

    //angulo de incidencia do raio no solo
    double theta = atan(d1/ht);

    //Calculo do coeficiente de reflex�o (Gamma) complexo
    std::complex<double> Gamma (0.0,0.0);
    double cos_theta = cos(theta);
    double sin_theta = sin(theta);
    std::complex<double> e_r (epsilon_r,(-1)*sigma*60*lambda.get());//-sigma*60*lambda.get()
    std::complex<double> cos_theta_complex (cos_theta, 0.0);
    std::complex<double> sen_theta_complex (sin_theta, 0.0);


    const std::complex<double> sqrt_term = sqrt(e_r - sen_theta_complex*sen_theta_complex);
    switch (polarization)
    {
        case 'h':
            // equation 4.25 in Rappaport (Gamma orthogonal)
            Gamma = (cos_theta_complex - sqrt_term) / (cos_theta_complex + sqrt_term);
            break;
        case 'v':
            // equation 4.24 in Rappaport (Gamma parallel)
            Gamma = (-e_r * cos_theta_complex + sqrt_term) / (e_r * cos_theta_complex + sqrt_term);
            break;
        default:
            throw cRuntimeError("Unknown polarization");
    }

    //Fator de perda de espalhamento
    double delta_fase = 2*beta*delta_h*cos(theta);
    double rho_s = exp((-0.5)*pow(delta_fase,2));
    std::complex<double> rho_s_complexo (rho_s,0.0); //passando rho_s para complexo

    std::complex<double> D_complexo (D,0.0);   //passando D para complexo

    std::complex<double> z ((cos(phi)/(r_r)),((-1)*sin(phi))/(r_r));
    std::complex<double> unitario (1/r_d,0.0);
    std::complex<double> a = unitario + D*Gamma*z*rho_s_complexo;

    std::complex<double> b;
    b = abs(a); //modulo de a b = |1+Gamma*z*D*rho_s|

    double c = real(b);
    double interference_term = c*c;//interference_term = |1+Gamma*z*D*rho_s|^2

    //friss_term  = (lambda/d*pi*d)^2
    const double friis_term = squared(lambda.get() / (4.0 * M_PI));//mudei de d_los para d(d_ground) - deletei o d

    return friis_term * interference_term;
}


double TwoRayNewModel::terraPlana(const Coord& pos_t, const Coord& pos_r, m lambda) const
{
    double Req = 1.33*6371000;

    double beta = 2*M_PI/(lambda.get()); //constante de fase
    double h_t = pos_t.z; //altura do transmissor em rela��o ao solo
    double h_r = pos_r.z;//altura do receptor em rela��o ao solo
    double d_x_t = pos_t.x;
    double d_x_r = pos_r.x;
    double d_y_t = pos_t.y;
    double d_y_r = pos_r.y;

    double d_los = pos_r.distance(pos_t);//sqrt(squared(d_x_t-d_x_r)+squared(d_y_t-d_y_r)+squared(h_t-h_r)); //distancia do raio direto - calculei duas vezes - depois limpar codigo
    double d = sqrt((d_los*d_los)-(h_t-h_r)*(h_t-h_r)); //distancia entre as antenas sob o solo
    double ht = h_t;
    double hr = h_r;


    double d1 = (h_t*d)/(h_t+h_r); //distancia do transmissor ao ponto de reflex�o
    double d2 = (h_r*d)/(h_t+h_r); //distancia do receptor ao potno de reflex�o

    double r_d = sqrt((d*d)+(h_t-h_r)*(h_t-h_r)); //trajeto do raio direto
    double r_r = sqrt((d*d)+(h_t+h_r)*(h_t+h_r)); //trajeto do raio refletido
    double dr = r_r - r_d; // diferen�a do refletido para o direto

    // Adjusted value -> crashed when transmitter on ground division by 0
    double theta;
    if (ht == 0) {
        theta = M_PI/2;
    } else {
       theta = atan(d1/ht);
    }

    double phi;
    if(d == 0){
        phi = atan(1);

    } else{
        phi = beta*(dr);
    }
    double D = 1; //fator de divergencia unit�rio para o caso da terra plana


    std::complex<double> Gamma (0.0,0.0);
    double cos_theta = cos(theta);
    double sin_theta = sin(theta);
    std::complex<double> e_r (epsilon_r,(-1)*sigma*60*lambda.get());//-sigma*60*lambda.get()
    std::complex<double> cos_theta_complex (cos_theta, 0.0);
    std::complex<double> sen_theta_complex (sin_theta, 0.0);


    //Calculo do coeficiente de reflex�o (Gamma) complexo
    const std::complex<double> sqrt_term = sqrt(e_r - sen_theta_complex*sen_theta_complex);
    switch (polarization)
    {
        case 'h':
            // equation 4.25 in Rappaport (Gamma orthogonal)
            Gamma = (cos_theta_complex - sqrt_term) / (cos_theta_complex + sqrt_term);
            break;
        case 'v':
            // equation 4.24 in Rappaport (Gamma parallel)
            Gamma = (-e_r * cos_theta_complex + sqrt_term) / (e_r * cos_theta_complex + sqrt_term);
            break;
        default:
            throw cRuntimeError("Unknown polarization");
    }

    double delta_fase = 2*beta*delta_h*cos(theta);
    double rho_s = exp((-0.5)*pow(delta_fase,2));
    std::complex<double> rho_s_complexo (rho_s,0.0); //passando rho_s para complexo

    std::complex<double> z ((cos(phi)/(r_r)),((-1)*sin(phi))/(r_r));
    std::complex<double> unitario (1/r_d,0.0); //acrescentei o r_d
    std::complex<double> a = unitario + Gamma*z*rho_s_complexo;

    /*
     * Adicionando onda de superficie
     *  a = unitario + Gamma*z   +(unitario-Gamma)*z*A
     *  std::complex<double> A = (((2*M_PI*d)/lambda.get())*(sen_theta_complex+sqrt_term)*(sen_theta_complex+sqrt_term))/(unitario+(((2*M_PI*d)/lambda.get())*((2*M_PI*d)/lambda.get()))*(sen_theta_complex+sqrt_term)*(sen_theta_complex+sqrt_term)*(sen_theta_complex+sqrt_term)*(sen_theta_complex+sqrt_term));
     */

    std::complex<double> b;
    b = abs(a); //modulo de a b = |1+Gamma*z|
    double c = real(b);
    //interference_term = |1+Gamma*z|^2
    double interference_term = c*c;

    //friss_term  = (lambda/d*pi*d)^2
    const double friis_term = squared(lambda.get() / (4.0 * M_PI));
    return friis_term * interference_term;

}

} // namespace gradys_simulations
