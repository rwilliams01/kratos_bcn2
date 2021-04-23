//    |  /           | 
//    ' /   __| _` | __|  _ \   __| 
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/ 
//                   Multi-Physics  
//
//  License:		 BSD License 
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//












// System includes
#include <string>
#include <iostream>
#include <vector>

// External includes

// Project includes
#include "includes/define.h"
#include "includes/kratos_application.h"
#include "includes/legacy_structural_app_vars.h"


namespace Kratos
{
    KRATOS_CREATE_VARIABLE( double, LAMBDA_OLD )
    KRATOS_CREATE_VARIABLE( double, LAMBDA_NULL )
    KRATOS_CREATE_VARIABLE( double, LAMBDA_EINS )
    KRATOS_CREATE_VARIABLE( double, LAMBDA_DT )
    KRATOS_CREATE_VARIABLE( double, LAMBDA_NULL_DT )
    KRATOS_CREATE_VARIABLE( double, LAMBDA_EINS_DT )
    KRATOS_CREATE_VARIABLE( double, LAMBDA_DT2 )
    KRATOS_CREATE_VARIABLE( double, LAMBDA_NULL_DT2 )
    KRATOS_CREATE_VARIABLE( double, LAMBDA_EINS_DT2 )
    KRATOS_CREATE_VARIABLE( Matrix, LAMBDAS_T )
    KRATOS_CREATE_VARIABLE( Matrix, DELTA_LAMBDAS_T )
    KRATOS_CREATE_VARIABLE( Matrix, CONTACT_LINK_M )
    KRATOS_CREATE_VARIABLE( Matrix, AUXILIARY_MATRIX_1 )
    KRATOS_CREATE_VARIABLE( Matrix, ELASTIC_LEFT_CAUCHY_GREEN_OLD )

    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( DISPLACEMENT_OLD )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( DISPLACEMENT_DT )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( DISPLACEMENT_NULL )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( DISPLACEMENT_NULL_DT )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( ACCELERATION_NULL )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( DISPLACEMENT_EINS )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( DISPLACEMENT_EINS_DT )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( ACCELERATION_EINS )

    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( ROTATION_OLD )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( ROTATION_DT )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( ROTATION_NULL )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( ROTATION_NULL_DT )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( ANGULAR_ACCELERATION_NULL )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( ROTATION_EINS )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( ROTATION_EINS_DT )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( ANGULAR_ACCELERATION_EINS )

    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( FORCE_NULL )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( FORCE_EINS )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( FACE_LOAD_NULL )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS( FACE_LOAD_EINS )

    KRATOS_CREATE_VARIABLE( Vector, PENALTY_T )
    KRATOS_CREATE_VARIABLE( double, INITIAL_PENALTY_T )
    KRATOS_CREATE_VARIABLE( Vector, LAMBDAS )
    KRATOS_CREATE_VARIABLE( Vector, GAPS )
    KRATOS_CREATE_VARIABLE( Vector, DELTA_LAMBDAS )
    KRATOS_CREATE_VARIABLE( Vector, STICK )
    KRATOS_CREATE_VARIABLE( double, CONTACT_STICK )

    KRATOS_CREATE_VARIABLE( bool, USE_DISTRIBUTED_PROPERTIES )

    KRATOS_CREATE_VARIABLE( int, CONTACT_RAMP )
    KRATOS_CREATE_VARIABLE( int, CONTACT_SLAVE_INTEGRATION_POINT_INDEX )
    KRATOS_CREATE_VARIABLE( int, CONTACT_DOUBLE_CHECK )
//    KRATOS_CREATE_VARIABLE( int, FIRST_TIME_STEP )
//    KRATOS_CREATE_VARIABLE( int, QUASI_STATIC_ANALYSIS )

    KRATOS_CREATE_VARIABLE( int, INTEGRATION_ORDER )
    KRATOS_CREATE_VARIABLE( double, INTEGRATION_WEIGHT )
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS(INTEGRATION_POINT_GLOBAL)
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS(INTEGRATION_POINT_GLOBAL_IN_REFERENCE_CONFIGURATION)
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS(INTEGRATION_POINT_GLOBAL_IN_CURRENT_CONFIGURATION)
    KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS(INTEGRATION_POINT_LOCAL)

//    KRATOS_CREATE_VARIABLE( int, OSS_SWITCH )
    KRATOS_CREATE_VARIABLE( int, WRINKLING_APPROACH )
    KRATOS_CREATE_VARIABLE( int, CALCULATE_INSITU_STRESS )
//    KRATOS_CREATE_VARIABLE( int, PERIODIC_PAIR_INDEX )
//    KRATOS_CREATE_VARIABLE( int, STATIONARY )

//    KRATOS_CREATE_VARIABLE( int, PARTITION_MASK )

//    KRATOS_CREATE_VARIABLE( double, FACE_HEAT_FLUX )

//    KRATOS_CREATE_VARIABLE( double, NODAL_VOLUME )

    KRATOS_CREATE_VARIABLE( double, PRESSURE_DT )
    KRATOS_CREATE_VARIABLE( double, WATER_PRESSURE_DT )
    KRATOS_CREATE_VARIABLE( double, WATER_PRESSURE_ACCELERATION )
    KRATOS_CREATE_VARIABLE( double, WATER_PRESSURE_NULL )
    KRATOS_CREATE_VARIABLE( double, WATER_PRESSURE_NULL_DT )
    KRATOS_CREATE_VARIABLE( double, WATER_PRESSURE_NULL_ACCELERATION )
    KRATOS_CREATE_VARIABLE( double, WATER_PRESSURE_EINS )
    KRATOS_CREATE_VARIABLE( double, WATER_PRESSURE_EINS_DT )
    KRATOS_CREATE_VARIABLE( double, WATER_PRESSURE_EINS_ACCELERATION )
    KRATOS_CREATE_VARIABLE( double, REACTION_LAGRANGE_WATER_PRESSURE )

    KRATOS_CREATE_VARIABLE( double, AIR_PRESSURE_DT )
    KRATOS_CREATE_VARIABLE( double, AIR_PRESSURE_ACCELERATION )
    KRATOS_CREATE_VARIABLE( double, AIR_PRESSURE_NULL )
    KRATOS_CREATE_VARIABLE( double, AIR_PRESSURE_NULL_DT )
    KRATOS_CREATE_VARIABLE( double, AIR_PRESSURE_NULL_ACCELERATION )
    KRATOS_CREATE_VARIABLE( double, AIR_PRESSURE_EINS )
    KRATOS_CREATE_VARIABLE( double, AIR_PRESSURE_EINS_DT )
    KRATOS_CREATE_VARIABLE( double, AIR_PRESSURE_EINS_ACCELERATION )
    KRATOS_CREATE_VARIABLE( double, REACTION_LAGRANGE_AIR_PRESSURE )
    KRATOS_CREATE_VARIABLE( double, SUCTION )
//    KRATOS_CREATE_VARIABLE( double, FLAG_VARIABLE )

//    KRATOS_CREATE_VARIABLE( double, DP_EPSILON )
//    KRATOS_CREATE_VARIABLE( double, DP_ALPHA1 )
//    KRATOS_CREATE_VARIABLE( double, DP_K )

//    KRATOS_CREATE_VARIABLE( double, EQ_STRAIN_RATE )
//    KRATOS_CREATE_VARIABLE( double, RHS_WATER )
//    KRATOS_CREATE_VARIABLE( double, RHS_AIR )

    KRATOS_CREATE_VARIABLE( double, PERMEABILITY_28_DAYS )
    KRATOS_CREATE_VARIABLE( double, PERMEABILITY_1_DAY )
    KRATOS_CREATE_VARIABLE( double, PERMEABILITY_TRANSITION )

    KRATOS_CREATE_VARIABLE( double, TEMPERATURE_DT )
    KRATOS_CREATE_VARIABLE( double, TEMPERATURE_DT_DT )
//    KRATOS_CREATE_VARIABLE( double, TEMPERATURE_OLD_IT )
//    KRATOS_CREATE_VARIABLE( double, EFFECTIVE_VISCOSITY )
//    KRATOS_CREATE_VARIABLE( double, KINEMATIC_VISCOSITY)
//    KRATOS_CREATE_VARIABLE( double, DYNAMIC_VISCOSITY)
//    KRATOS_CREATE_VARIABLE( double, WEIGHT_FATHER_NODES )

    KRATOS_CREATE_VARIABLE( int, PARENT_ELEMENT_ID )
    KRATOS_CREATE_VARIABLE( int, INTEGRATION_POINT_INDEX )

    KRATOS_CREATE_VARIABLE( int, IS_CONTACT_MASTER )
    KRATOS_CREATE_VARIABLE( int, IS_CONTACT_SLAVE )
//    KRATOS_CREATE_VARIABLE( double, IS_BOUNDARY )
//    KRATOS_CREATE_VARIABLE( double, IS_VISITED )

    KRATOS_CREATE_VARIABLE( double, LAGRANGE_MULTIPLIER_CONSTRAINT )
    KRATOS_CREATE_VARIABLE( double, LAGRANGE_MULTIPLIER_CONSTRAINT_REACTION )

    KRATOS_CREATE_VARIABLE( int, IS_SHAPE_FUNCTION_REQUIRED )
    KRATOS_CREATE_VARIABLE( int, RESET_CONFIGURATION )

    void KratosApplication::RegisterLegacyStructuralAppVariables()
    {
        KRATOS_REGISTER_VARIABLE( LAMBDA_OLD )
        KRATOS_REGISTER_VARIABLE( LAMBDA_NULL )
        KRATOS_REGISTER_VARIABLE( LAMBDA_EINS )
        KRATOS_REGISTER_VARIABLE( LAMBDA_DT )
        KRATOS_REGISTER_VARIABLE( LAMBDA_NULL_DT )
        KRATOS_REGISTER_VARIABLE( LAMBDA_EINS_DT )
        KRATOS_REGISTER_VARIABLE( LAMBDA_DT2 )
        KRATOS_REGISTER_VARIABLE( LAMBDA_NULL_DT2 )
        KRATOS_REGISTER_VARIABLE( LAMBDA_EINS_DT2 )
        KRATOS_REGISTER_VARIABLE( LAMBDAS_T )
        KRATOS_REGISTER_VARIABLE( DELTA_LAMBDAS_T )
        KRATOS_REGISTER_VARIABLE( CONTACT_LINK_M )
        KRATOS_REGISTER_VARIABLE( AUXILIARY_MATRIX_1 )
        KRATOS_REGISTER_VARIABLE( ELASTIC_LEFT_CAUCHY_GREEN_OLD )

        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( DISPLACEMENT_OLD )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( DISPLACEMENT_DT )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( DISPLACEMENT_NULL )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( DISPLACEMENT_NULL_DT )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( ACCELERATION_NULL )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( DISPLACEMENT_EINS )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( DISPLACEMENT_EINS_DT )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( ACCELERATION_EINS )

        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( ROTATION_OLD )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( ROTATION_DT )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( ROTATION_NULL )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( ROTATION_NULL_DT )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( ANGULAR_ACCELERATION_NULL )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( ROTATION_EINS )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( ROTATION_EINS_DT )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( ANGULAR_ACCELERATION_EINS )

        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( FORCE_NULL )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( FORCE_EINS )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( FACE_LOAD_NULL )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( FACE_LOAD_EINS )

        KRATOS_REGISTER_VARIABLE( PENALTY_T )
        KRATOS_REGISTER_VARIABLE( INITIAL_PENALTY_T )
        KRATOS_REGISTER_VARIABLE( LAMBDAS )
        KRATOS_REGISTER_VARIABLE( GAPS )
        KRATOS_REGISTER_VARIABLE( DELTA_LAMBDAS )
        KRATOS_REGISTER_VARIABLE( STICK )
        KRATOS_REGISTER_VARIABLE( CONTACT_STICK )

        KRATOS_REGISTER_VARIABLE( USE_DISTRIBUTED_PROPERTIES )

        KRATOS_REGISTER_VARIABLE( CONTACT_RAMP )
        KRATOS_REGISTER_VARIABLE( CONTACT_SLAVE_INTEGRATION_POINT_INDEX )
        KRATOS_REGISTER_VARIABLE( CONTACT_DOUBLE_CHECK )
//        KRATOS_REGISTER_VARIABLE( FIRST_TIME_STEP )
//        KRATOS_REGISTER_VARIABLE( QUASI_STATIC_ANALYSIS )

        KRATOS_REGISTER_VARIABLE( INTEGRATION_ORDER )
        KRATOS_REGISTER_VARIABLE( INTEGRATION_WEIGHT )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( INTEGRATION_POINT_GLOBAL )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( INTEGRATION_POINT_GLOBAL_IN_REFERENCE_CONFIGURATION )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( INTEGRATION_POINT_GLOBAL_IN_CURRENT_CONFIGURATION )
        KRATOS_REGISTER_3D_VARIABLE_WITH_COMPONENTS( INTEGRATION_POINT_LOCAL )

//        KRATOS_REGISTER_VARIABLE( OSS_SWITCH )
        KRATOS_REGISTER_VARIABLE( WRINKLING_APPROACH )
        KRATOS_REGISTER_VARIABLE( CALCULATE_INSITU_STRESS )
//        KRATOS_REGISTER_VARIABLE( PERIODIC_PAIR_INDEX )
//        KRATOS_REGISTER_VARIABLE( STATIONARY )

//        KRATOS_REGISTER_VARIABLE( PARTITION_MASK )

//        KRATOS_REGISTER_VARIABLE( FACE_HEAT_FLUX )

//        KRATOS_REGISTER_VARIABLE( NODAL_VOLUME )

        KRATOS_REGISTER_VARIABLE( PRESSURE_DT )
        KRATOS_REGISTER_VARIABLE( WATER_PRESSURE_DT )
        KRATOS_REGISTER_VARIABLE( WATER_PRESSURE_ACCELERATION )
        KRATOS_REGISTER_VARIABLE( WATER_PRESSURE_NULL )
        KRATOS_REGISTER_VARIABLE( WATER_PRESSURE_NULL_DT )
        KRATOS_REGISTER_VARIABLE( WATER_PRESSURE_NULL_ACCELERATION )
        KRATOS_REGISTER_VARIABLE( WATER_PRESSURE_EINS )
        KRATOS_REGISTER_VARIABLE( WATER_PRESSURE_EINS_DT )
        KRATOS_REGISTER_VARIABLE( WATER_PRESSURE_EINS_ACCELERATION )
        KRATOS_REGISTER_VARIABLE( REACTION_LAGRANGE_WATER_PRESSURE )

        KRATOS_REGISTER_VARIABLE( AIR_PRESSURE_DT )
        KRATOS_REGISTER_VARIABLE( AIR_PRESSURE_ACCELERATION )
        KRATOS_REGISTER_VARIABLE( AIR_PRESSURE_NULL )
        KRATOS_REGISTER_VARIABLE( AIR_PRESSURE_NULL_DT )
        KRATOS_REGISTER_VARIABLE( AIR_PRESSURE_NULL_ACCELERATION )
        KRATOS_REGISTER_VARIABLE( AIR_PRESSURE_EINS )
        KRATOS_REGISTER_VARIABLE( AIR_PRESSURE_EINS_DT )
        KRATOS_REGISTER_VARIABLE( AIR_PRESSURE_EINS_ACCELERATION )
        KRATOS_REGISTER_VARIABLE( REACTION_LAGRANGE_AIR_PRESSURE )
        KRATOS_REGISTER_VARIABLE( SUCTION )
//        KRATOS_REGISTER_VARIABLE( FLAG_VARIABLE )

//        KRATOS_REGISTER_VARIABLE( DP_EPSILON )
//        KRATOS_REGISTER_VARIABLE( DP_ALPHA1 )
//        KRATOS_REGISTER_VARIABLE( DP_K )

//        KRATOS_REGISTER_VARIABLE( EQ_STRAIN_RATE )
//        KRATOS_REGISTER_VARIABLE( RHS_WATER )
//        KRATOS_REGISTER_VARIABLE( RHS_AIR )

        KRATOS_REGISTER_VARIABLE( PERMEABILITY_28_DAYS )
        KRATOS_REGISTER_VARIABLE( PERMEABILITY_1_DAY )
        KRATOS_REGISTER_VARIABLE( PERMEABILITY_TRANSITION )

//        KRATOS_REGISTER_VARIABLE( TEMPERATURE_OLD_IT )
//        KRATOS_REGISTER_VARIABLE( EFFECTIVE_VISCOSITY )
//        KRATOS_REGISTER_VARIABLE( KINEMATIC_VISCOSITY)
//        KRATOS_REGISTER_VARIABLE( DYNAMIC_VISCOSITY)
//        KRATOS_REGISTER_VARIABLE( WEIGHT_FATHER_NODES )

        KRATOS_REGISTER_VARIABLE( PARENT_ELEMENT_ID )
        KRATOS_REGISTER_VARIABLE( INTEGRATION_POINT_INDEX )

        KRATOS_REGISTER_VARIABLE( IS_CONTACT_MASTER )
        KRATOS_REGISTER_VARIABLE( IS_CONTACT_SLAVE )
//        KRATOS_REGISTER_VARIABLE( IS_BOUNDARY )
//        KRATOS_REGISTER_VARIABLE( IS_VISITED )

        KRATOS_REGISTER_VARIABLE( LAGRANGE_MULTIPLIER_CONSTRAINT )
        KRATOS_REGISTER_VARIABLE( LAGRANGE_MULTIPLIER_CONSTRAINT_REACTION )

        KRATOS_REGISTER_VARIABLE( IS_SHAPE_FUNCTION_REQUIRED )
        KRATOS_REGISTER_VARIABLE( RESET_CONFIGURATION )
    }

}  // namespace Kratos.

// This CREATE must be HERE
#undef DKRATOS_EXPORT_INTERFACE_2



