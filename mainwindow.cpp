#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMouseEvent"
#include "qpainter.h"
#include "qbrush.h"
#include "qcolor.h"
#include "QInputDialog"
extern "C"
{
#include "triangle.h"
}
#include "qdebug.h"
#include "qsettings.h"
#include "qfiledialog.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionOpen,SIGNAL(triggered(bool)),this,SLOT(onOpen()));
    connect(ui->actionSave,SIGNAL(triggered(bool)),this,SLOT(onSave()));
    connect(ui->actionExit,SIGNAL(triggered(bool)),this,SLOT(onExit()));
    connect(ui->actionRun,SIGNAL(triggered(bool)),this,SLOT(onRun()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onOpen()
{
    QString filename=QFileDialog::getOpenFileName(this,tr("open"),",",QString("All(*.*)"));
    if(filename.isEmpty())
        return;
    vertices.clear();
    QSettings setting(filename,QSettings::IniFormat);
    for(QString key:setting.allKeys())
    {
        QPointF p= setting.value(key).toPointF();
        vertices.append(p);
    }
}

void MainWindow::onSave()
{
    QString filename=QFileDialog::getSaveFileName(this,tr("save"),",",QString("All(*.*)"));
    if(filename.isEmpty())
        return;
    QSettings setting(filename,QSettings::IniFormat);
    setting.clear();
    for(int i=0;i<vertices.size();i++)
    {
        setting.setValue(QString("%1").arg(i),
                         vertices[i]);
    }
}

void MainWindow::onExit()
{
}

void MainWindow::onRun()
{
    if(vertices.size()<3)return;
    QInputDialog qid;
    QString switchs=qid.getText(this,tr("flags"),"flags",QLineEdit::Normal,"pqa100");
    if(switchs.size()==0)return;
    if(!switchs.contains('z'))switchs.append('z');
    std::string css(switchs.toStdString().c_str());
    char*flags=(char*)css.c_str();
    triangulateio in,out,vout;
    memset(&in,0,sizeof(in));
    memset(&out,0,sizeof(out));
    memset(&vout,0,sizeof(vout));

    in.numberofpoints=vertices.size();
    QVector<REAL> inpoints(vertices.size()*2);
    in.pointlist=inpoints.data();
    QVector<int> inmarker(vertices.size(),3);
    in.pointmarkerlist=inmarker.data();
    for(int i=0;i<vertices.size();i++)
    {
        in.pointlist[i*2]=vertices[i].x();
        in.pointlist[i*2+1]=vertices[i].y();
    }
    QVector<int> inedges(vertices.size()*2);
    in.segmentlist=inedges.data();
    int ei=-1;
    for(int i=0;i<vertices.size();i++)
    {
        inedges[++ei]=i;
        inedges[++ei]=(i+1)%vertices.size();
    }
    in.numberofsegments=vertices.size();
    QVector<int> insegmentmarker(vertices.size(),0);
    for(int i=0;i<insegmentmarker.size();i++)
    {
        insegmentmarker[i]=i;
    }
    in.segmentmarkerlist=insegmentmarker.data();

    qDebug()<<"in:";
    qDebug()<<"points size:"<<in.numberofpoints;

    triangulate(flags,&in,&out,&vout);
    if(out.numberoftriangles>0)
    {
        triangles.resize(out.numberoftriangles);
        memcpy(&triangles[0],out.trianglelist
                ,sizeof(vec3)*triangles.size());
        delete[] out.trianglelist;
    }
    if(out.numberofpoints>0)
    {
        QMap<int,int> mmm;
        points.resize(out.numberofpoints);
        for(int i=0;i<points.size();i++)
        {
            points[i].setX(out.pointlist[i*2]);
            points[i].setY(out.pointlist[i*2+1]);
            if(mmm.contains(out.pointmarkerlist[i]))
            {
                mmm[out.pointmarkerlist[i]]++;
            }
            else {
                mmm[out.pointmarkerlist[i]]=1;
            }
        }
        for(int key:mmm.keys())
        {
            QString kfirst=QString("%1:%2").arg(tr("marker")).arg(key);
            QString vfirst=QString("%1:%2").arg(tr("number")).arg(mmm[key]);
            qDebug()<<kfirst<<","<<vfirst;
        }
    }
    qDebug()<<"out:";
    qDebug()<<"points size:"<<out.numberofpoints;

    update();
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    auto check=[&](const float&a,float esp=1e-6)
    {
        return std::abs(a)<esp;
    };
//    auto collisionCheck=[&](){
//        for(int i=0;i<vertices.size();i++)
//        {
//            if(check(e->pos().x()-vertices[i].x(),0.05)||
//                    check(e->pos().y()-vertices[i].y(),0.05))
//            {
//                return true;
//            }
//        }
//        return false;
//    };
//    ///检测碰撞点
//    if(collisionCheck())
//    {
//        return;
//    }
    /// 存入内存
    vertices.push_back(e->pos());
    update();
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::black,2));
    painter.drawPoints(vertices.data(),vertices.size());

    if(triangles.size()>0)
    {
        if(points.size()>0)
        {
            for(int i=0;i<triangles.size();i++)
            {
                vec3 t=triangles[i];
                QPolygonF tri;
                tri.append(points[t.a]);
                tri.append(points[t.b]);
                tri.append(points[t.c]);
                painter.drawPolygon(tri);
            }
        }
    }
}


