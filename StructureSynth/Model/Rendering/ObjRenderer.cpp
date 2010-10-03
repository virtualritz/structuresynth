#include "ObjRenderer.h"
#include "SyntopiaCore/Logging/Logging.h"

using namespace SyntopiaCore::Math;
using namespace SyntopiaCore::GLEngine;
using namespace SyntopiaCore::Logging;

namespace StructureSynth {
	namespace Model {
		namespace Rendering {


			void ObjGroup::addGroup(ObjGroup g) {
				int vertexCount = vertices.count();
				int normalCount = normals.count();
				
				for (int i = 0; i < g.vertices.count(); i++) {
					vertices.append(g.vertices[i]);
				}
				
				for (int i = 0; i < g.normals.count(); i++) {
					normals.append(g.normals[i]);
				}
				
				for (int i = 0; i < g.faces.count(); i++) {
					for (int j = 0; j < g.faces[i].count(); j++) {
						g.faces[i][j].vID = g.faces[i][j].vID + vertexCount;
						g.faces[i][j].nID = g.faces[i][j].nID + normalCount;
					}
					faces.append(g.faces[i]);
				}
			}

			// Removes redundant vertices. 
			void ObjGroup::reduceVertices() {
				QVector<Vector3f> newVertices;
				QVector<Vector3f> newNormals;
				QMap<int,int> oldToNewVertex;
				QMap<int,int> oldToNewNormals;

				// Reduce vertices and normals (if present)
				for (int i = 0; i < vertices.count(); i++) {
					int index = newVertices.indexOf(vertices[i]);
					if (index==-1) {
						newVertices.append(vertices[i]);
						index=newVertices.count()-1;
					}
					oldToNewVertex[i] = index;
				}

				// Reduce vertices and normals (if present)
				for (int i = 0; i < normals.count(); i++) {
					int index = newNormals.indexOf(normals[i]);
					if (index==-1) {
						newNormals.append(normals[i]);
						index=newNormals.count()-1;
					}
					oldToNewNormals[i] = index;
				}


				// Update indices
				for (int i = 0; i < faces.count(); i++) {
					for (int j = 0; j < faces[i].count(); j++) {
						faces[i][j].vID = oldToNewVertex[faces[i][j].vID-1]+1; // beware OBJ is 1-based!
						faces[i][j].nID = oldToNewNormals[faces[i][j].nID-1]+1; 
					}
				}

				vertices = newVertices;
				normals = newNormals;
			}

			namespace {
				/*
				This function was taken from Paul Bourkes website:
				http://local.wasp.uwa.edu.au/~pbourke/miscellaneous/sphere_cylinder/

				Create a unit sphere centered at the origin
				This code illustrates the concept rather than implements it efficiently
				It is called with two arguments, the theta and phi angle increments in degrees
				Note that at the poles only 3 vertex facet result
				while the rest of the sphere has 4 point facets
				*/
				void CreateUnitSphere(int dtheta,int dphi, ObjGroup& motherGroup, Matrix4f m)
				{
					float DTOR = 3.1415/180.0;
					ObjGroup group;
					for (int theta=-90;theta<=90-dtheta;theta+=dtheta) {
						for (int phi=0;phi<=360-dphi;phi+=dphi) {
							int vi = group.vertices.count()+1;
							int vn = group.normals.count()+1;

							group.vertices.append(m*Vector3f(cos(theta*DTOR) * cos(phi*DTOR), cos(theta*DTOR) * sin(phi*DTOR), sin(theta*DTOR)));
							group.normals.append(Vector3f(cos(theta*DTOR) * cos(phi*DTOR), cos(theta*DTOR) * sin(phi*DTOR), sin(theta*DTOR)));

							QVector<VertexNormal> vns; 
							if (theta > -90 && theta < 90) {
								group.vertices.append(m*Vector3f( cos(theta*DTOR) * cos((phi+dphi)*DTOR),cos(theta*DTOR) * sin((phi+dphi)*DTOR),  sin(theta*DTOR)));
								group.normals.append(Vector3f( cos(theta*DTOR) * cos((phi+dphi)*DTOR),cos(theta*DTOR) * sin((phi+dphi)*DTOR),  sin(theta*DTOR)));
								for (int j = 0; j<4; j++) vns.append(VertexNormal(j+vi,j+vn));
							} else {
								for (int j = 0; j<3; j++) vns.append(VertexNormal(j+vi,j+vn));
							}
							group.vertices.append(m*Vector3f( cos((theta+dtheta)*DTOR) * cos((phi+dphi)*DTOR),	cos((theta+dtheta)*DTOR) * sin((phi+dphi)*DTOR), sin((theta+dtheta)*DTOR)));
							group.normals.append(Vector3f( cos((theta+dtheta)*DTOR) * cos((phi+dphi)*DTOR),	cos((theta+dtheta)*DTOR) * sin((phi+dphi)*DTOR), sin((theta+dtheta)*DTOR)));
							group.vertices.append(m*Vector3f(cos((theta+dtheta)*DTOR) * cos(phi*DTOR), cos((theta+dtheta)*DTOR) * sin(phi*DTOR), sin((theta+dtheta)*DTOR)));
							group.normals.append(Vector3f(cos((theta+dtheta)*DTOR) * cos(phi*DTOR), cos((theta+dtheta)*DTOR) * sin(phi*DTOR), sin((theta+dtheta)*DTOR)));
							group.faces.append(vns);
						}
					}
					group.reduceVertices();
					motherGroup.addGroup(group);
				}

			}


			void ObjRenderer::addQuad(ObjGroup& group, Vector3f v1,Vector3f v2,Vector3f v3,Vector3f v4) {
				int vi = group.vertices.count()+1;
				int vn = group.normals.count()+1;
				group.vertices.append(v1);
				group.vertices.append(v2);
				group.vertices.append(v3);
				group.vertices.append(v4);

				Vector3f normal = Vector3f::cross((v2-v1), (v4-v1)).normalized();
				group.normals.append(normal); 
				group.normals.append(normal);
				group.normals.append(normal);
				group.normals.append(normal);
				
				QVector<VertexNormal> vns; 
				for (int j = 0; j<4; j++) vns.append(VertexNormal(vi+j, vn+j));
				group.faces.append(vns);
			}

			void ObjRenderer::setClass(QString classID, Vector3f /*rgb*/, double /*alpha*/) {

				QString className = classID; // TODO: add in rgb+alpga?
				if (className.isEmpty()) className = "default";
				if (!groups.contains(className)) groups[className] = ObjGroup();
				groups[className].groupName = className;
				currentGroup = className;
			}


			void ObjRenderer::drawBox(SyntopiaCore::Math::Vector3f O, 
				SyntopiaCore::Math::Vector3f v1 , 
				SyntopiaCore::Math::Vector3f v2, 
				SyntopiaCore::Math::Vector3f v3, PrimitiveClass* classID) {
					setClass(classID->name,rgb,alpha);
					ObjGroup group;
					addQuad(group, O, O+v2,O+v2+v1,O+v1);
					addQuad(group, O+v3, O+v1+v3, O+v2+v1+v3, O+v2+v3);
					addQuad(group, O, O+v3, O+v3+v2,O+v2);
					addQuad(group, O+v1,O+v2+v1, O+v3+v2+v1,  O+v3+v1);
					addQuad(group, O, O+v1, O+v3+v1, O+v3);
					addQuad(group, O+v2, O+v3+v2 , O+v3+v2+v1,O+v1+v2);
					group.reduceVertices();
					groups[currentGroup].addGroup(group);
			};


			void ObjRenderer::drawMesh(  SyntopiaCore::Math::Vector3f startBase, 
				SyntopiaCore::Math::Vector3f startDir1, 
				SyntopiaCore::Math::Vector3f startDir2, 
				SyntopiaCore::Math::Vector3f endBase, 
				SyntopiaCore::Math::Vector3f endDir1, 
				SyntopiaCore::Math::Vector3f endDir2, 
				PrimitiveClass* classID) {


			};

			void ObjRenderer::drawGrid(SyntopiaCore::Math::Vector3f O, 
				SyntopiaCore::Math::Vector3f v1 , 
				SyntopiaCore::Math::Vector3f v2, 
				SyntopiaCore::Math::Vector3f v3,
				PrimitiveClass* classID) {
					setClass(classID->name,rgb,alpha);
					ObjGroup group;
					addQuad(group,O, O+v2,O+v2+v1,O+v1);
					addQuad(group,O+v3, O+v1+v3, O+v2+v1+v3, O+v2+v3);
					addQuad(group,O, O+v3, O+v3+v2,O+v2);
					addQuad(group,O+v1,O+v2+v1, O+v3+v2+v1,  O+v3+v1);
					addQuad(group,O, O+v1, O+v3+v1, O+v3);
					addQuad(group,O+v2, O+v3+v2 , O+v3+v2+v1,O+v1+v2);
					group.reduceVertices();
					groups[currentGroup].addGroup(group);
			};

			void ObjRenderer::drawLine(SyntopiaCore::Math::Vector3f from, SyntopiaCore::Math::Vector3f to, PrimitiveClass* classID) {
			};

			void ObjRenderer::drawTriangle(SyntopiaCore::Math::Vector3f p1,
				SyntopiaCore::Math::Vector3f p2,
				SyntopiaCore::Math::Vector3f p3,
				PrimitiveClass* classID) {
			}

			void ObjRenderer::drawDot(SyntopiaCore::Math::Vector3f v, PrimitiveClass* classID) {
			};

			void ObjRenderer::drawSphere(SyntopiaCore::Math::Vector3f center, float radius, PrimitiveClass* classID) {
				setClass(classID->name,rgb,alpha);

				Matrix4f m  = Matrix4f::Translation(center.x(),center.y(),center.z())*(Matrix4f::ScaleMatrix(radius));

				CreateUnitSphere(30,30,groups[currentGroup],m);
			};

			void ObjRenderer::begin() {
				rgb = Vector3f(1,0,0);
				alpha = 1;
			};

			void ObjRenderer::end() {
			};


			void ObjRenderer::writeToStream(QTextStream& ts) {
				int vertexCount = 0;
				int normalCount = 0;

				foreach (ObjGroup o, groups) {
					// Group name
					INFO(o.groupName);
					ts << "g " << o.groupName << endl;
					ts << "usemtl " << o.groupName << endl;

					// Vertices
					foreach (Vector3f v, o.vertices) {
						ts << "v "	<< QString::number(v.x()) << " "
							<< QString::number(v.y())  << " "
							<< QString::number(v.z())  << " "
							<< endl;
					}

					// Normals
					foreach (Vector3f v, o.normals) {
						ts << "vn "	<< QString::number(v.x()) << " "
							<< QString::number(v.y())  << " "
							<< QString::number(v.z())  << " "
							<< endl;
					}

					// Faces
					foreach (QVector<VertexNormal> vi, o.faces) {
						ts << "f ";
						
						foreach (VertexNormal vn, vi) ts << QString::number(vn.vID+vertexCount) << "//"
								<< QString::number(vn.nID+normalCount) << " ";
						ts << endl;
					}
					vertexCount += o.vertices.count();
					normalCount += o.normals.count();
				}
			};




		}
	}
}
