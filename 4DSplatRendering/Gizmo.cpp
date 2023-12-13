#include "Gizmo.h"

void Gizmo::DrawLine(glm::vec3 from, glm::vec3 to)
{
	glLineWidth(5.0);
	glBegin(GL_LINES);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(from.x, from.y, from.z);
	glVertex3f(to.x, to.y, to.z);
	glEnd();
}
