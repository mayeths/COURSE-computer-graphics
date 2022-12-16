#include"light.h"
#include<sstream>
#include<string>
#include<cmath>
#include<cstdlib>
#define ran() ( double( rand() % 32768 ) / 32768 )

Light::Light() {
	sample = rand();
	next = NULL;
	lightPrimitive = NULL;
}

void Light::Input( std::string var , std::stringstream& fin ) {
	if ( var == "color=" ) color.Input( fin );
}

void PointLight::Input( std::string var , std::stringstream& fin ) {
	if ( var == "O=" ) O.Input( fin );
	Light::Input( var , fin );
}


double PointLight::CalnShade( Vector3 C , Primitive* primitive_head , int shade_quality ) {
	// C是物体上给定的一个碰撞点，应该是遍历所有可能的点传进来
	Vector3 V = O - C; // 光源到这个碰撞点的方向
	double dist = V.Module();
	for ( Primitive* now = primitive_head ; now != NULL ; now = now->GetNext() )
	{
		CollidePrimitive tmp = now->Collide(C, V); // 给定光源和光的方向，计算是否碰撞
		if ( EPS < (dist - tmp.dist) )  return 0;
	}

	return 1;
}

void SquareLight::Input( std::string var , std::stringstream& fin ) {
	if ( var == "O=" ) O.Input( fin );
	if ( var == "Dx=" ) Dx.Input( fin );
	if ( var == "Dy=" ) Dy.Input( fin );
	Light::Input( var , fin );
}


double SquareLight::CalnShade( Vector3 C , Primitive* primitive_head , int shade_quality ) {
	// Vector3 O; Vector3 Dx, Dy;
	// 我令O为中心，Dx和Dy是边长。
	int shade = 0;
	int n = 4 * shade_quality;
	int ni = (int)sqrt((double)n);
	int nj = n / ni;
	for (int i = 0; i < ni; i++) {
		for (int j = 0; j < nj; j++) {
			Vector3 pointLightPos = Dx * ((i-ni/2)/ni) + Dy * ((j-nj/2)/nj);
			Vector3 V = O - C + pointLightPos;
			double dist = V.Module();

			for (Primitive* now = primitive_head; now != NULL; now = now->GetNext()) {
				CollidePrimitive tmp = now->Collide(C, V);
				if (tmp.isCollide) {
					if (tmp.dist - dist < -EPS) {
						shade++;
						break;
					}
				}
			}
		}
	}
	return 1 - (double)shade / (double)(ni*nj);
}

Primitive* SquareLight::CreateLightPrimitive()
{
	PlaneAreaLightPrimitive* res = new PlaneAreaLightPrimitive(O, Dx, Dy, color);
	lightPrimitive = res;
	return res;
}



void SphereLight::Input( std::string var , std::stringstream& fin ) {
	if ( var == "O=" ) O.Input( fin );
	if ( var == "R=" ) fin>>R;
	Light::Input( var , fin );
}


double SphereLight::CalnShade( Vector3 C , Primitive* primitive_head , int shade_quality ) {
	int shade = 0;
	int n = 4 * shade_quality;
	int ni = (int)cbrt(n);
	int nj = (int)cbrt(n);
	int nk = n / (ni * nj);
	// 用6面立方体到球体映射的这个公式：https://math.stackexchange.com/q/118760 (cube to sphere)
	for (int i = 0; i < ni; i++) {
		for (int j = 0; j < nj; j++) {
			for (int k = 0; k < nk; k++) {
				const double cx = (double)i;
				const double cy = (double)j;
				const double cz = (double)k;
				const double sx = cx * sqrt(1 - (cy*cy)/2 - (cz*cz)/2 + (cy*cy*cz*cz)/3);
				const double sy = cy * sqrt(1 - (cz*cz)/2 - (cx*cx)/2 + (cz*cz*cx*cx)/3);
				const double sz = cz * sqrt(1 - (cx*cx)/2 - (cy*cy)/2 + (cx*cx*cy*cy)/3);
				Vector3 pointLightPos(sx, sy, sz);
				Vector3 V = O - C + pointLightPos;
				double dist = V.Module();

				for (Primitive* now = primitive_head; now != NULL; now = now->GetNext()) {
					CollidePrimitive tmp = now->Collide(C, V);
					if (tmp.isCollide) {
						if (tmp.dist - dist < -EPS) {
							shade++;
							break;
						}
					}
				}
			}
		}
	}
	return 1 - (double)shade / (double)(ni*nj*nk);
}


Primitive* SphereLight::CreateLightPrimitive()
{
	SphereLightPrimitive* res = new SphereLightPrimitive(O, R, color);
	lightPrimitive = res;
	return res;
}

