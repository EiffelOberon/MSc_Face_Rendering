#define PI 3.1415f
#include <iostream>

static inline float degToRad(float angle) {  return (angle*PI)/180.f; }


static void printMatrix(glm::mat4 mat){
	for(int i=0; i<4; i++){
		for(int j=0; j<4; j++){
			std::cout<<mat[i][j]<<" ";
		}
		printf("\n");
	}
}