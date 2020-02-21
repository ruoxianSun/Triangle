#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}
struct vec3
{
    union{
        struct {
            int a,b,c;
        };
        int data[3];
    };
};
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected slots:
    void onOpen();
    void onSave();
    void onExit();
    void onRun();
protected:
    void mousePressEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *e);
private:
    QVector<QPointF> vertices;
    QVector<vec3> triangles;
    QVector<QPointF> points;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
