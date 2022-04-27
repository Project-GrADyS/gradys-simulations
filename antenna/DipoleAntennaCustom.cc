//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "DipoleAntennaCustom.h"

//.h contem informacao de cabecalho q vai ser usada em varios arquivos e o .cc /.cpp
//apenas implementacao local


namespace projeto {
Define_Module(DipoleAntennaCustom);
DipoleAntennaCustom::DipoleAntennaCustom() :
    AntennaBase()
{
}

void DipoleAntennaCustom::initialize(int stage)
{
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        double maxGain = math::dB2fraction(par("maxGain"));
        double minGain = math::dB2fraction(par("minGain"));
        const char *wireAxis = par("wireAxis");
        m length = m(par("length"));
        m lambda = m(par("lambda"));

        gain = makeShared<AntennaGain>(wireAxis, length, lambda, maxGain, minGain);
    }
    // par retorna referencia para um parametro espeficado com esse nome
}

std::ostream& DipoleAntennaCustom::printToStream(std::ostream& stream, int level) const
{
    stream << "DipoleAntenna";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", length = " << gain->getLength();
        stream << ", maxGain = " << gain->getMaxGain();
        stream << ", minGain = " << gain->getMinGain();
        stream << ", lambda = " << gain->getLambda();
    return AntennaBase::printToStream(stream, level);
}

DipoleAntennaCustom::AntennaGain::AntennaGain(const char *wireAxis, m length, m lambda, double maxGain, double minGain) :
        maxGain(maxGain), minGain(minGain), length(length), lambda(lambda)
{
    wireAxisDirection = Coord::parse(wireAxis);
}

double DipoleAntennaCustom::AntennaGain::computeGain(Quaternion direction) const
{
    double angle = std::acos(direction.rotate(Coord::X_AXIS) * wireAxisDirection);
    double a = std::acos(0.0);
    double pi = 2*a;
    double comp_ele = (length.get()/lambda.get());
    double t = (std::cos(pi*comp_ele*std::cos(angle))-std::cos(pi*comp_ele))/std::sin(angle);
    double q = t/ (1-std::cos(pi*comp_ele));
    return maxGain * q * q;
//q = sin(angle)
}
}
