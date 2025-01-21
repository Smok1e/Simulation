#pragma once

//======================================

constexpr int R1 =  8, G1 =  6, B1 =  6;
constexpr int R2 = 11, G2 =  9, B2 = 11;
constexpr int R3 = 11, G3 = 11, B3 =  9;
constexpr int R  =  1, G  =  2, B  =  2;

constexpr auto TRANSACTION1_AVG_DELAY               = R1;
constexpr auto TRANSACTION2_AVG_DELAY               = G1;
constexpr auto TRANSACTION3_AVG_DELAY               = B1;
constexpr auto TRANSACTION1_AVG_PROCESSING_TIME     = R1 + G1 + B1;
constexpr auto TRANSACTION2_AVG_PROCESSING_TIME     = R2 + G2 + B2;
constexpr auto TRANSACTION3_AVG_PROCESSING_TIME_OP1 = R3 + G3 + B3 + G1;
constexpr auto TRANSACTION3_AVG_PROCESSING_TIME_OP2 = R3 + G3 + B3 + B1;
											     
constexpr int DEFAULT_SIMULATION_DURATION   = 1000;
constexpr int DEFAULT_AVG_TRANSACTION_QUEUE = 3;
constexpr int DEFAULT_MAX_QUEUE_SIZE        = 7;
constexpr int DEFAULT_DELAY_MS              = 100;
constexpr int DEFAULT_OP1_COUNT             = 5;
constexpr int DEFAULT_OP2_COUNT             = 5;

//======================================
