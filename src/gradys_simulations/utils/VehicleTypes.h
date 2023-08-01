/*
 * VehicleTypes.h
 *
 *  Created on: 12 de mar de 2022
 *      Author: thlam
 */

#ifndef gradys_simulations_UTILS_VEHICLETYPES_H_
#define gradys_simulations_UTILS_VEHICLETYPES_H_


namespace gradys_simulations {

enum VehicleType: unsigned int {
    COPTER = 1,
    PLANE = 1<<1,
    ROVER = 1<<2
};

}


#endif /* gradys_simulations_UTILS_VEHICLETYPES_H_ */
