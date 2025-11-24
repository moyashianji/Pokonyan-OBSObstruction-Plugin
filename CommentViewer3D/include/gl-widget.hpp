#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QVector3D>

class PhysicsEngine;

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit GLWidget(PhysicsEngine* physicsEngine, QWidget* parent = nullptr);
    ~GLWidget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void DrawGround();
    void DrawComments();
    void DrawText(const QString& text, float x, float y, float z, float scale = 1.0f);
    void RenderTexturedQuad(float width, float height);

    PhysicsEngine* m_physicsEngine;

    // Camera settings
    QVector3D m_cameraPos;
    QVector3D m_cameraTarget;
    QVector3D m_cameraUp;
    float m_cameraDistance;
    float m_cameraAngleX;
    float m_cameraAngleY;

    // Mouse interaction
    QPoint m_lastMousePos;
    bool m_mousePressed;

    // Projection matrices
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;
};
