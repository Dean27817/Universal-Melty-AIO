
#ifndef KINIMATICS
#define KINIMATICS
class kinimatics
{
	public:
	// Kinematics: curentRads = robot heading, translationSpeed = [0–1], translationRads = desired move direction
	float *getSpeed(float curentRads, float translationSpeed, float translationRads, float spinSpeed) {

	  float LeftRight[2] = {0, 0};

	  // Offset term relative to heading
	  double offset = sin(curentRads - translationRads) * (1.0 - spinSpeed) * translationSpeed;

	  // Base spin plus offset
	  LeftRight[1] = spinSpeed + offset;
	  LeftRight[0] = spinSpeed - offset;

	  // Clamp between -1 and 1
	  LeftRight[0] = constrain(LeftRight[0], -1.0, 1.0);
	  LeftRight[1] = constrain(LeftRight[1], -1.0, 1.0);

	  float *returnptr = LeftRight;
	  return returnptr;
	}
};
#endif
