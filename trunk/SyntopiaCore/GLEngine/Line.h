#pragma once

#include "SyntopiaCore/Math/Vector3.h"
#include "Object3D.h"

namespace SyntopiaCore {
	namespace GLEngine {	

		class Line : public Object3D {
		public:
			Line(SyntopiaCore::Math::Vector3f from, SyntopiaCore::Math::Vector3f to);

			virtual ~Line();

			virtual void draw();

		private:
			SyntopiaCore::Math::Vector3f from;
			SyntopiaCore::Math::Vector3f to;
		};

	}
}