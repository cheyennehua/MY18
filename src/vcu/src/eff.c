#include "eff.h"

uint8_t const data_eff_percent[NUM_SPD_INDXS][NUM_TRQ_INDXS] = {
  { 46, 72, 82, 87, 63, 22, 27, 27, 24, 47, 18, 26},
  { 83, 70, 97, 99, 99, 100, 96, 100, 100, 84, 84, 100},
  { 70, 90, 95, 96, 100, 100, 98, 100, 100, 92, 100, 100},
  { 73, 77, 92, 91, 91, 94, 94, 94, 96, 96, 96, 96},
  { 74, 69, 78, 86, 92, 89, 89, 89, 90, 90, 90, 91},
  { 75, 74, 82, 85, 75, 87, 91, 91, 92, 92, 92, 94},
  { 79, 60, 96, 83, 87, 86, 87, 99, 99, 86, 86, 86},
  { 75, 82, 85, 81, 56, 76, 84, 79, 79, 84, 84, 88},
  { 73, 82, 81, 84, 85, 100, 82, 81, 81, 83, 83, 84},
  { 74, 84, 82, 82, 85, 90, 85, 99, 84, 84, 84, 85},
  { 75, 79, 80, 82, 78, 78, 83, 81, 72, 79, 59, 63},
  { 73, 89, 80, 84, 85, 84, 84, 85, 93, 83, 83, 74},
  { 71, 84, 81, 79, 80, 84, 83, 84, 84, 88, 80, 89},
  { 75, 81, 83, 80, 83, 83, 86, 85, 82, 87, 84, 78},
  { 73, 83, 81, 80, 80, 84, 83, 82, 86, 86, 78, 82},
  { 58, 82, 81, 80, 80, 80, 87, 87, 80, 87, 83, 82},
  { 64, 82, 82, 81, 81, 82, 82, 84, 82, 79, 84, 82},
  { 68, 79, 81, 81, 80, 80, 83, 81, 82, 80, 87, 84},
  { 57, 78, 78, 77, 78, 82, 81, 80, 81, 82, 89, 81},
  { 60, 78, 77, 76, 78, 79, 79, 80, 82, 81, 82, 80},
  { 70, 79, 77, 77, 78, 80, 79, 79, 77, 80, 80, 82},
  { 68, 81, 80, 77, 80, 80, 78, 79, 79, 77, 77, 80},
  { 60, 79, 79, 78, 79, 76, 78, 82, 81, 77, 77, 77},
  { 64, 79, 77, 77, 75, 76, 79, 76, 81, 77, 78, 77},
  { 60, 74, 72, 75, 76, 78, 79, 78, 79, 75, 77, 75},
  { 53, 70, 76, 75, 73, 75, 77, 76, 79, 75, 75, 77},
  { 52, 51, 67, 74, 74, 75, 74, 71, 77, 71, 74, 68},
  { 51, 25, 58, 63, 64, 71, 57, 70, 71, 66, 65, 63},
  { 39, 18, 42, 37, 52, 58, 58, 64, 68, 63, 62, 50},
  { 39, 22, 38, 42, 63, 61, 53, 53, 50, 53, 43, 39},
  { 55, 60, 41, 65, 54, 42, 42, 38, 42, 34, 34, 32},
  { 51, 67, 58, 43, 63, 37, 22, 38, 38, 27, 29, 26},
  { 47, 48, 59, 54, 57, 40, 31, 33, 32, 26, 29, 24},
  { 37, 44, 58, 36, 50, 43, 22, 28, 25, 25, 25, 24},
  { 46, 64, 27, 41, 41, 38, 21, 22, 21, 23, 24, 24},
  { 59, 48, 40, 47, 41, 37, 27, 22, 22, 22, 23, 23},
  { 51, 57, 53, 37, 44, 36, 25, 20, 22, 23, 24, 23},
  { 55, 28, 14, 47, 35, 28, 24, 22, 23, 21, 24, 21},
  { 44, 56, 42, 42, 39, 32, 23, 20, 21, 21, 21, 21},
  { 58, 58, 53, 39, 37, 24, 23, 20, 21, 18, 19, 21},
  { 58, 53, 62, 35, 31, 24, 22, 23, 19, 19, 19, 21},
  { 35, 23, 16, 18, 25, 21, 23, 24, 21, 23, 21, 21},
  { 44, 11, 9, 17, 21, 21, 20, 19, 20, 21, 21, 24},
  { 17, 24, 21, 17, 20, 20, 19, 20, 20, 19, 19, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
  { 14, 14, 16, 20, 17, 19, 20, 20, 20, 20, 20, 20},
};
