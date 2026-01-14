#include "gl-widget.hpp"
#include "physics-engine.hpp"
#include "comment-3d.hpp"
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>

GLWidget::GLWidget(PhysicsEngine* physicsEngine, QWidget* parent)
    : QOpenGLWidget(parent)
    , m_physicsEngine(physicsEngine)
    , m_cameraPos(0, 20, 50)
    , m_cameraTarget(0, 0, 0)
    , m_cameraUp(0, 1, 0)
    , m_cameraDistance(50.0f)
    , m_cameraAngleX(30.0f)
    , m_cameraAngleY(0.0f)
    , m_mousePressed(false)
{
}

GLWidget::~GLWidget() {
}

void GLWidget::initializeGL() {
    initializeOpenGLFunctions();

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Set light properties
    GLfloat lightPosition[] = { 10.0f, 50.0f, 30.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    // Enable color material
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Set background to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Anti-aliasing
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void GLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);

    // Setup projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = static_cast<float>(w) / static_cast<float>(h);
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 500.0f;

    float f = 1.0f / tan(fov * 3.14159f / 360.0f);
    float rangeInv = 1.0f / (nearPlane - farPlane);

    float matrix[16] = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (nearPlane + farPlane) * rangeInv, -1,
        0, 0, nearPlane * farPlane * rangeInv * 2, 0
    };
    glMultMatrixf(matrix);

    glMatrixMode(GL_MODELVIEW);
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Update camera position based on angles
    float radX = m_cameraAngleX * 3.14159f / 180.0f;
    float radY = m_cameraAngleY * 3.14159f / 180.0f;

    m_cameraPos.setX(m_cameraDistance * cos(radX) * sin(radY));
    m_cameraPos.setY(m_cameraDistance * sin(radX));
    m_cameraPos.setZ(m_cameraDistance * cos(radX) * cos(radY));

    // Setup camera
    gluLookAt(
        m_cameraPos.x(), m_cameraPos.y(), m_cameraPos.z(),
        m_cameraTarget.x(), m_cameraTarget.y(), m_cameraTarget.z(),
        m_cameraUp.x(), m_cameraUp.y(), m_cameraUp.z()
    );

    // Draw ground
    DrawGround();

    // Draw comments
    DrawComments();
}

void GLWidget::DrawGround() {
    glDisable(GL_LIGHTING);

    // Draw grid
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_LINES);
    for (int i = -50; i <= 50; i += 5) {
        // Lines parallel to X axis
        glVertex3f(-50.0f, 0.0f, static_cast<float>(i));
        glVertex3f(50.0f, 0.0f, static_cast<float>(i));

        // Lines parallel to Z axis
        glVertex3f(static_cast<float>(i), 0.0f, -50.0f);
        glVertex3f(static_cast<float>(i), 0.0f, 50.0f);
    }
    glEnd();

    // Draw ground plane
    glColor4f(0.1f, 0.1f, 0.1f, 1.0f);
    glBegin(GL_QUADS);
    glVertex3f(-50.0f, -0.1f, -50.0f);
    glVertex3f(50.0f, -0.1f, -50.0f);
    glVertex3f(50.0f, -0.1f, 50.0f);
    glVertex3f(-50.0f, -0.1f, 50.0f);
    glEnd();

    glEnable(GL_LIGHTING);
}

void GLWidget::DrawComments() {
    if (!m_physicsEngine) {
        return;
    }

    const auto& comments = m_physicsEngine->GetComments();
    for (const auto& comment : comments) {
        Comment3DRenderer::RenderComment(comment);
    }
}

void GLWidget::DrawText(const QString& text, float x, float y, float z, float scale) {
    // Text rendering will be implemented later with texture-based rendering
    // For now, we just draw the white boxes
}

void GLWidget::RenderTexturedQuad(float width, float height) {
    float w = width / 2.0f;
    float h = height / 2.0f;

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-w, -h, 0);
    glTexCoord2f(1, 0); glVertex3f(w, -h, 0);
    glTexCoord2f(1, 1); glVertex3f(w, h, 0);
    glTexCoord2f(0, 1); glVertex3f(-w, h, 0);
    glEnd();
}

void GLWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_mousePressed = true;
        m_lastMousePos = event->pos();
    }
}

void GLWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_mousePressed) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();

        m_cameraAngleY += delta.x() * 0.5f;
        m_cameraAngleX += delta.y() * 0.5f;

        // Clamp vertical angle
        if (m_cameraAngleX > 89.0f) m_cameraAngleX = 89.0f;
        if (m_cameraAngleX < -89.0f) m_cameraAngleX = -89.0f;

        update();
    }
}

void GLWidget::wheelEvent(QWheelEvent* event) {
    float delta = event->angleDelta().y() / 120.0f;
    m_cameraDistance -= delta * 3.0f;

    if (m_cameraDistance < 5.0f) m_cameraDistance = 5.0f;
    if (m_cameraDistance > 200.0f) m_cameraDistance = 200.0f;

    update();
}
