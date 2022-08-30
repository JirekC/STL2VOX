#ifndef F3D_MATH_HPP
#define F3D_MATH_HPP

#include "glm/glm.hpp"

namespace f3d {

    /**
	 * \brief Generates rotation matrix
	 *
	 * \param rot   Tait-Bryan rotation ( rot{ alpha, beta, gamma }, where: \b alpha around \b X3, \b beta around \b Y2, \b gamma around \b Z1 ) in [rad].
     *              Please note reversed order of individual rotations: first, object is rotated around \b Z than \b Y' and at the end around \b X''.
	 * \return      transformation matrix for Mat x Vec multiply
	 */
	inline glm::mat4 RotationMatrix(const glm::vec3& rot) {
		glm::mat4 res;

		float c1 = cos(rot.z);
		float c2 = cos(rot.y);
		float c3 = cos(rot.x);
		float s1 = sin(rot.z);
		float s2 = sin(rot.y);
		float s3 = sin(rot.x);
        // row 1
		res[0][0] = c1 * c2;
		res[1][0] = c1 * s2 * s3 - c3 * s1;
		res[2][0] = s1 * s3 + c1 * s2 * c3;
        res[3][0] = 0.0f;
        // row 2
		res[0][1] = s1 * c2;
		res[1][1] = c1 * c3 + s1 * s2 * s3;
		res[2][1] = s1 * s2 * c3 - c1 * s3;
        res[3][1] = 0.0f;
        //row 3
		res[0][2] = -s2;
		res[1][2] = c2 * s3;
		res[2][2] = c2 * c3;
        res[3][2] = 0.0f;
        //row 4
        res[0][3] = 0.0f;
        res[1][3] = 0.0f;
        res[2][3] = 0.0f;
        res[3][3] = 1.0f;

		return res;
	}

}

#endif
