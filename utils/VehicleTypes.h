/*
 * VehicleTypes.h
 *
 *  Created on: 12 de mar de 2022
 *      Author: thlam
 */

#ifndef PROJETO_UTILS_VEHICLETYPES_H_
#define PROJETO_UTILS_VEHICLETYPES_H_


namespace projeto {

enum VehicleType: unsigned int {
    COPTER = 1,
    PLANE = 1<<1,
    ROVER = 1<<2
};

}


#endif /* PROJETO_UTILS_VEHICLETYPES_H_ */
