/*
 *  MainWindow.h
 *  
 *  Author: Ching-Hsiang (Tom) Hsu
 *  Email: chhsu@nyu.edu
 *  Modified: Ching-Hsiang (Tom) Hsu, Oct. 31, 2016
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>

#include "Timer.h"
#include "Triangle.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //void showAngleBound(double t1_lowerBound, double t1_upperBound, double t2_lowerBound, double t2_upperBound);


    // Print text
    virtual MainWindow& operator<<(const std::string&);
    virtual MainWindow& operator<<(const char*);
    virtual MainWindow& operator<<(int);
    virtual MainWindow& operator<<(long);
    virtual MainWindow& operator<<(float);
    virtual MainWindow& operator<<(double);

private slots:
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);



    void on_randomButton_clicked();

    void on_bfs_clicked();
    void on_greedy_clicked();
    void on_dist_clicked();
    void on_vor_clicked();

    void on_run_clicked();
    void on_exit_clicked();

    void on_showTrace_clicked();
    void on_showPath_clicked();
    void on_showFilledObstacles_clicked();
    void on_showRobot_clicked();
    void on_showAnim_clicked();
    void on_pauseAnim_clicked();
    void on_replayAnim_clicked();

    void on_hideBox_clicked();
    void on_hideBoxBoundary_clicked();
    void on_safe_angle_clicked();

    void on_non_crossing_clicked();

    void on_pushButton_clicked();

    void on_inc_valueChanged(int arg1);
    void on_left_clicked();
    void on_right_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_saveCfg_clicked();


private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
