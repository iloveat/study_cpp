#include <stdio.h>
#include <GL/glut.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <vector>

using namespace std;


typedef struct MyPoint3f
{
    float x;
    float y;
    float z;
    MyPoint3f(float xx, float yy, float zz)
    {
        x = xx;
        y = yy;
        z = zz;
    }
}
MyPoint3f;


GLint Width = 752, Height = 480;
GLfloat xangle = 0, yangle = 0;
GLfloat oldx, oldy;
vector<MyPoint3f> point3d_front;
vector<MyPoint3f> point3d_back;


void InitGL()
{
    glShadeModel(GL_SMOOTH); // Enables Smooth Shading
    glClearColor(0.0, 0.0, 0.0, 0.0); // Black Background
    glClearDepth(1.0); // Depth Buffer Setup
    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); // The Type Of Depth Test To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective Calculations
}

void glTest()
{
    glPointSize(3);
    glBegin(GL_POINTS);
        glVertex3f(20, 20, 20);
    glEnd();

    glBegin(GL_QUADS); // Draw A Quad
        glColor3f(1.0f, 0.0f, 0.0f); // Set The Color To Red
        glVertex3f(-10.0f, 50.0f, 0.0f); // Top Left
        glColor3f(0.0f, 1.0f, 0.0f); // Set The Color To Green
        glVertex3f( 10.0f, 50.0f, 0.0f); // Top Right
        glColor3f(0.0f, 0.0f, 1.0f); // Set The Color To Blue
        glVertex3f( 10.0f,-50.0f, 0.0f); // Bottom Right
        glColor3f(1.0f, 1.0f, 0.0f); // Set The Color To Red
        glVertex3f(-10.0f,-50.0f, 0.0f); // Bottom Left
    glEnd(); // Done Drawing The Quad

    glTranslatef(50.0f,0.0f,-30.0f); // Move Right 3 Units
    glBegin(GL_TRIANGLES); // Drawing Using Triangles
        glColor3f(1.0f, 1.0f, 1.0f); // Set The Color To Red
        glVertex3f(-10.0f, 10.0f, 0.0f); // Top
        glVertex3f(-10.0f,-10.0f, 0.0f); // Bottom Left
        glVertex3f( 10.0f,-10.0f, 30.0f); // Bottom Right
    glEnd(); // Finished Drawing The Triangle
}

void draw3dFace()
{
    glRotatef(180, 1.0, 0.0, 0.0);
    MyPoint3f nose = point3d_front[30];
    glTranslatef(-nose.x, -nose.y, -nose.z); // Move to nose

    glPointSize(3);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POINTS);
        for(unsigned int i = 0; i < point3d_front.size(); i++)
        {
            MyPoint3f pt = point3d_front[i];
            glVertex3f(pt.x, pt.y, pt.z);
        }
    glEnd();

    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
        for(unsigned int i = 0; i < 16; i++)
        {
            MyPoint3f pt1 = point3d_front[i];
            glVertex3f(pt1.x, pt1.y, pt1.z);
            MyPoint3f pt2 = point3d_front[i+1];
            glVertex3f(pt2.x, pt2.y, pt2.z);
        }
    glEnd();

    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
        for(unsigned int i = 17; i < 21; i++)
        {
            MyPoint3f pt1 = point3d_front[i];
            glVertex3f(pt1.x, pt1.y, pt1.z);
            MyPoint3f pt2 = point3d_front[i+1];
            glVertex3f(pt2.x, pt2.y, pt2.z);
        }
    glEnd();

    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
        for(unsigned int i = 22; i < 26; i++)
        {
            MyPoint3f pt1 = point3d_front[i];
            glVertex3f(pt1.x, pt1.y, pt1.z);
            MyPoint3f pt2 = point3d_front[i+1];
            glVertex3f(pt2.x, pt2.y, pt2.z);
        }
    glEnd();

    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
        for(unsigned int i = 27; i < 30; i++)
        {
            MyPoint3f pt1 = point3d_front[i];
            glVertex3f(pt1.x, pt1.y, pt1.z);
            MyPoint3f pt2 = point3d_front[i+1];
            glVertex3f(pt2.x, pt2.y, pt2.z);
        }
    glEnd();

    glColor3f(1.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
        for(unsigned int i = 31; i < 35; i++)
        {
            MyPoint3f pt1 = point3d_front[i];
            glVertex3f(pt1.x, pt1.y, pt1.z);
            MyPoint3f pt2 = point3d_front[i+1];
            glVertex3f(pt2.x, pt2.y, pt2.z);
        }
    glEnd();

    glPolygonMode(GL_FRONT, GL_LINE);
    glPolygonMode(GL_BACK, GL_LINE);
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_POLYGON);
        for(unsigned int i = 36; i < 41; i++)
        {
            MyPoint3f pt1 = point3d_front[i];
            glVertex3f(pt1.x, pt1.y, pt1.z);
        }
    glEnd();

    glBegin(GL_POLYGON);
        for(unsigned int i = 42; i < 47; i++)
        {
            MyPoint3f pt1 = point3d_front[i];
            glVertex3f(pt1.x, pt1.y, pt1.z);
        }
    glEnd();

    glColor3f(0.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);
        for(unsigned int i = 48; i < 59; i++)
        {
            MyPoint3f pt1 = point3d_front[i];
            glVertex3f(pt1.x, pt1.y, pt1.z);
        }
    glEnd();

    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_POLYGON);
        for(unsigned int i = 60; i < 67; i++)
        {
            MyPoint3f pt1 = point3d_front[i];
            glVertex3f(pt1.x, pt1.y, pt1.z);
        }
    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear The Screen And The Depth Buffer
    glPushMatrix();
    glRotatef(xangle, 1.0, 0.0, 0.0);
    glRotatef(yangle, 0.0, 1.0, 0.0);
    glScalef(4, 4, 6);

    //glTest();
    draw3dFace();

    glPopMatrix();
    glutSwapBuffers();

    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        oldx = x;
        oldy = y;
    }
}

void motion(int x, int y)
{
    // 根据屏幕上鼠标滑动的距离来设置旋转的角度
    GLint deltax = oldx - x;
    GLint deltay = oldy - y;
    yangle += 360 * (GLfloat)deltax / (GLfloat)Width;
    xangle += 360 * (GLfloat)deltay / (GLfloat)Height;
    // 更新鼠标坐标
    oldx = x;
    oldy = y;
    glutPostRedisplay();
}

void reshape(int width, int height)
{
    GLfloat nRange = 376.0f;
    if(0==height) height = 1; // Prevent A Divide By Zero

    glViewport(0, 0, (GLint)width, (GLint)height); // Reset The Current Viewport

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // Reset The Projection Matrix

    if (width <= height)
        glOrtho(-nRange, nRange, -nRange*height/width, nRange*height/width, -nRange, nRange);
    else
        glOrtho(-nRange*width/height, nRange*width/height, -nRange, nRange, -nRange, nRange);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

MyPoint3f parse_point(string strPoint)
{
    int first_comma = strPoint.find_first_of(',');
    int last_comma = strPoint.find_last_of(',');
    string strX = strPoint.substr(0, first_comma);
    string strY = strPoint.substr(first_comma+1, last_comma-first_comma-1);
    string strZ = strPoint.substr(last_comma+1);
    //cout<<strX<<endl<<strY<<endl<<strZ<<endl;
    MyPoint3f pt(atof(strX.c_str()), atof(strY.c_str()), atof(strZ.c_str()));
    return pt;
}

string parse_string(string str2process)
{
    while(str2process[0] != '$' && str2process.length() != 0)
    {
        int end_index = str2process.find_first_of(';');
        if(-1 == end_index)
        {
            //cout<<"end_index: "<<end_index<<endl;
            break;
        }

        string strPoint = str2process.substr(0, end_index);
        //cout<<strPoint<<endl;
        MyPoint3f pt = parse_point(strPoint);
        //cout<<pt.x<<endl<<pt.y<<endl<<pt.z<<endl;
        point3d_back.push_back(pt);

        str2process = str2process.substr(end_index+1);
    }
    string rest = str2process;
    if('$' == str2process[0])
    {
        rest = str2process.substr(1);
        if(68 == point3d_back.size())
        {
            point3d_front.swap(point3d_back);

            vector<MyPoint3f> vec;
            point3d_back.clear();
            point3d_back.swap(vec);
        }
        else
        {
            vector<MyPoint3f> vec;
            point3d_back.clear();
            point3d_back.swap(vec);
            cout<<"points size error"<<endl;
        }
    }
    return rest;
}

string get_rest(string str2process)
{
    string strTmp = str2process;
    string rest = "";
    while(rest.length() != str2process.length())
    {
        str2process = strTmp;
        strTmp = parse_string(str2process);
        rest = strTmp;
    }
    return rest;
}

void* onRecvMsg(void *arg)
{
    int cnfd = *(int*)arg;
    const int recv_buffer_size = 2048;
    char buff[recv_buffer_size];

    int ret;
    string str2process = "";
    while((ret = recv(cnfd, buff, recv_buffer_size, 0)) != -1)
    {
        if(ret == 0)
            break;
        //cout<<"recve:"<<buff<<endl;
        str2process.append(buff);
        str2process = get_rest(str2process);
    }

    close(cnfd);
    cout<<"one client left"<<endl;
    return (void*)0;
}

void* onAcceptClient(void *arg)
{
    int skfd = *(int*)arg;
    cout<<"start listening"<<endl;

    while(true)
    {
        int cnfd = accept(skfd, NULL, NULL);
        if(-1 == cnfd)
        {
            perror("accept error");
            close(cnfd);
            break;
        }

        cout<<"one client connected"<<endl;
        pthread_t thread;
        if(pthread_create(&thread, NULL, onRecvMsg, &cnfd) != 0)
        {
            perror("pthread_create onRecvMsg error");
            close(cnfd);
            continue;
        }
    }

    close(skfd);
    cout<<"stop listening"<<endl;
    return (void*)0;
}

void Test_parse_string()
{
    // test string parsing method
    string str2process = "7.5,49.9,29.0;7.2,62.2,29.5;8.5,74.6,30.0;$";
    str2process = "7.5,49.9,29.0;7.2,62.2,29.5;8.5,74.6,30.0;";
    str2process = "7.5,49.9,29.0;7.2,62.2,29.5;8.5,74.6,30.0";
    str2process = "7.5,49.9,29.0;7.2,62.2,29.5;8.5,74.";
    str2process = "7.5,49.9,29.0;7.2,62.2,29.5;8.5,74.6,30.0;$88.8,99.9";
    str2process = "";
    str2process = "7.5,49.9,29.0;7.2,62.2,29.5;8.5,74.6,30.0;$7.5,49.9,29.0;7.2,62.2,29.5;8.5,74.6,30.0;$7.5,49.9,29.";
    str2process = "9.9,55.4,2.2;10.4,67.6,4.1;10.9,79.9,0.0;13.5,92.1,1.8;17.6,103.7,3.7;25.3,114.7,5.7;35.3,122.1,10.9;48.2,127.3,16.2;61.8,129.0,16.0;75.5,128.5,15.9;87.1,123.7,16.3;95.2,115.2,14.0;101.1,104.5,11.9;104.2,93.1,9.9;106.6,80.1,11.1;106.8,67.9,9.2;106.3,56.4,7.3;25.2,39.8,18.4;30.4,32.7,21.9;39.1,28.5,22.2;49.2,28.7,22.1;58.5,32.5,24.7;69.3,32.7,27.6;77.3,29.2,27.8;86.6,30.1,24.7;94.5,33.9,21.3;97.9,41.8,20.6;64.0,46.3,26.4;63.8,54.9,25.6;64.4,63.5,27.8;65.0,72.2,30.0;54.0,80.6,26.4;59.1,81.4,26.2;64.1,83.0,26.1;69.1,81.6,26.1;73.5,81.0,26.2;33.7,48.6,17.5;38.8,45.1,20.7;45.3,45.2,20.7;51.0,49.6,23.2;45.2,51.0,23.1;38.7,50.9,20.2;74.0,50.8,20.0;79.8,46.5,23.3;86.3,46.7,23.2;90.5,51.1,19.8;86.2,52.4,22.7;79.7,52.3,22.7;47.2,100.0,21.7;53.8,95.0,25.1;59.6,93.0,28.2;63.9,94.5,25.0;68.2,93.1,25.1;73.2,96.1,24.8;77.4,100.5,24.4;72.4,102.6,21.3;68.0,103.2,21.2;63.7,103.8,24.2;58.7,103.0,24.3;53.6,102.2,24.4;50.1,99.3,21.7;59.5,97.3,27.8;63.8,97.4,24.8;68.1,97.4,24.7;75.3,99.7,24.5;68.1,98.2,24.7;63.8,98.1,24.7;59.5,98.0,27.7;$";

    string rest = get_rest(str2process);
    cout<<rest<<endl;

    // test point parsing method
    //string strPoint = "7.5,49.9,29.0";
    //MyPoint3f pt = parse_point(strPoint);
    //cout<<pt.x<<endl<<pt.y<<endl<<pt.z<<endl;
}


int main(int argc, char*argv[])
{
    Test_parse_string();

    // initial network
    int port = 51666;
    int skfd;
    sockaddr_in srv_addr;

    if(-1 == (skfd = socket(AF_INET, SOCK_STREAM, 0)))
    {
         perror("server socket error");
         exit(1);
    }

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = 0;
    srv_addr.sin_port = htons(port);

    if(-1 == bind(skfd, (sockaddr *)(&srv_addr), sizeof(sockaddr)))
    {
         perror("bind error");
         exit(1);
    }

    if(-1 == listen(skfd, 5))
    {
         perror("listen error");
         exit(1);
    }

    pthread_t thread;
    if(pthread_create(&thread, NULL, onAcceptClient, &skfd) != 0)
    {
        perror("pthread_create onAcceptClient error");
        exit(1);
    }

    // initial opengl
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(Width, Height);
    glutCreateWindow("FaceViewer");

    InitGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    glutMainLoop();
    return 0;
}
