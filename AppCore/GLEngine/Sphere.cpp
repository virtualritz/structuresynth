#include "Sphere.h"

using namespace AppCore::Math;


namespace AppCore {
	namespace GLEngine {


		//GLUquadric* Sphere::myQuad = 0;    

		Sphere::Sphere(AppCore::Math::Vector3f center, float radius) : center(center), radius(radius) {
			if (true) {
				myQuad = gluNewQuadric();    
				gluQuadricDrawStyle(myQuad, GLU_FILL);
			}
			/// Bounding box
			from = center-Vector3f(radius,radius,radius);
			to = center+Vector3f(radius,radius,radius);
		};

		Sphere::~Sphere() {
			gluDeleteQuadric(myQuad);
		};
 
		void Sphere::draw() {
			glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, primaryColor );
			if (primaryColor[3] < 1) {
				glEnable( GL_BLEND );
			}

			glPushMatrix();
			glTranslatef( center.x(), center.y(), center.z() );
			gluSphere(myQuad, radius, 7, 7);	
			glPopMatrix();			
		};

	}
}