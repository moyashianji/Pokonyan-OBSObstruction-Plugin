#include "comment-3d.hpp"
#include <QOpenGLFunctions>
#include <cmath>

void Comment3DRenderer::RenderComment(const Comment3DData& comment) {
    QOpenGLFunctions glFuncs(QOpenGLContext::currentContext());

    glFuncs.glPushMatrix();

    // Apply position
    glFuncs.glTranslatef(
        comment.position.x(),
        comment.position.y(),
        comment.position.z()
    );

    // Apply rotation
    btVector3 axis = comment.rotation.getAxis();
    btScalar angle = comment.rotation.getAngle();
    glFuncs.glRotatef(
        angle * 180.0f / 3.14159f,
        axis.x(),
        axis.y(),
        axis.z()
    );

    // Draw white cube
    glFuncs.glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    DrawCube(comment.size.x(), comment.size.y(), comment.size.z());

    // Draw text on the front face
    RenderText3D(comment.text, 0, 0, comment.size.z() / 2.0f + 0.01f);

    glFuncs.glPopMatrix();
}

void Comment3DRenderer::RenderText3D(const QString& text, float x, float y, float z) {
    // For now, we'll skip text rendering as it requires complex texture handling
    // This can be implemented later with QImage and OpenGL textures
}

void Comment3DRenderer::DrawCube(float width, float height, float depth) {
    QOpenGLFunctions glFuncs(QOpenGLContext::currentContext());

    float w = width / 2.0f;
    float h = height / 2.0f;
    float d = depth / 2.0f;

    glFuncs.glBegin(GL_QUADS);

    // Front face
    glFuncs.glNormal3f(0, 0, 1);
    glFuncs.glVertex3f(-w, -h, d);
    glFuncs.glVertex3f(w, -h, d);
    glFuncs.glVertex3f(w, h, d);
    glFuncs.glVertex3f(-w, h, d);

    // Back face
    glFuncs.glNormal3f(0, 0, -1);
    glFuncs.glVertex3f(-w, -h, -d);
    glFuncs.glVertex3f(-w, h, -d);
    glFuncs.glVertex3f(w, h, -d);
    glFuncs.glVertex3f(w, -h, -d);

    // Top face
    glFuncs.glNormal3f(0, 1, 0);
    glFuncs.glVertex3f(-w, h, -d);
    glFuncs.glVertex3f(-w, h, d);
    glFuncs.glVertex3f(w, h, d);
    glFuncs.glVertex3f(w, h, -d);

    // Bottom face
    glFuncs.glNormal3f(0, -1, 0);
    glFuncs.glVertex3f(-w, -h, -d);
    glFuncs.glVertex3f(w, -h, -d);
    glFuncs.glVertex3f(w, -h, d);
    glFuncs.glVertex3f(-w, -h, d);

    // Right face
    glFuncs.glNormal3f(1, 0, 0);
    glFuncs.glVertex3f(w, -h, -d);
    glFuncs.glVertex3f(w, h, -d);
    glFuncs.glVertex3f(w, h, d);
    glFuncs.glVertex3f(w, -h, d);

    // Left face
    glFuncs.glNormal3f(-1, 0, 0);
    glFuncs.glVertex3f(-w, -h, -d);
    glFuncs.glVertex3f(-w, -h, d);
    glFuncs.glVertex3f(-w, h, d);
    glFuncs.glVertex3f(-w, h, -d);

    glFuncs.glEnd();
}
