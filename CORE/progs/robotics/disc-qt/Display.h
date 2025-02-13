//==============================================================================
// Display.h
// =================
// This class is the widget on ControlWindow that renders the scene.
// Display generates and paints boxes, walls, and circles using OpenGL
// to allow the user to get a full understanding of the program's output.
//
// AUTHOR: Bryant Curto (bryantcurto@gmail.com)
// CREATED: 2015-07-28
//
// Copyright (c) 2015 Bryant Curto
//==============================================================================

#ifndef __disc__Display__
#define __disc__Display__

// Custom
#include "shader.h"
#include "ControlWindow.h"
#include "Box.h"
#include "QuadTree.h"


// Qt
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

// Standard Library
#include <vector>



class Display : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    Display(QWidget* parent = 0);
    virtual ~Display();
    
    void genScene();

    void setControlWindow(ControlWindow*);

    
protected:
    // Essential Functions Inherited from QOpenGLWidget
    virtual void initializeGL() Q_DECL_OVERRIDE;
    virtual void paintGL() Q_DECL_OVERRIDE;
    virtual void resizeGL(int width, int height) Q_DECL_OVERRIDE;
    
    shader* program;                    // Shader program used by OpenGL
    ControlWindow *controlWin;

    
    bool drawable;                      // Prevent rendering if error occurs
    int  filledCircleResolution = 32;   // Num vertices in filled circle
    int  hollowCircleResolution = 100;  // Num vertices in hollow circle
    
    
    /*********************************
     * Functions used to paint scene *
     *********************************/
    // Helper Functions
    void toNormalDeviceCoords(double& x, double& y);
    inline void setVertexColor(std::vector<double>& array, float r, float g, float b);
    void perpVertices(float thickness, std::vector<double>& storage, double a_x, double a_y,
                    double b_x, double b_y, float red, float green, float blue);
    // Generate Shapes
    void genPath(std::vector<Box*>& path, double* beta, double* alpha, double R0);
    void genLine(float thickness, double a_x, double a_y, double b_x, double b_y,
                 float red, float green, float blue, bool smoothCorners = true);
    void genQuad(Box* b, double epsilon);
    void genWalls(Box* b);
    void treeTraverse(Box* b, double epsilon);
    void treeTraverse_infSteps(Box* b, double epsilon);
    void treeTraverse_altSteps(Box* b, double epsilon, long maxSteps);
    void genHollowCircle(double radius, double x, double y,
                         float red, float green, float blue);
    void genFilledCircle(double radius, double x, double y,
                         float red, float green, float blue);
    // Paint Shapes to Screen
    void drawLines();
    void drawQuads();
    void drawCircles();
    void drawPath();
    void drawRobot(Box*);
};

#endif /* defined(__disc__Display__) */
