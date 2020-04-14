#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QXmlStreamReader>
#include <QFileDialog>
#include <QThreadPool>
#include <QOpenGLTexture>
#include <integrators/directlightingintegrator.h>
#include <integrators/naiveintegrator.h>
#include <integrators/fulllightingintegrator.h>
#include <scene/lights/diffusearealight.h>
#include <QDateTime>

#include <scene/geometry/cube.h>
#include <scene/geometry/sphere.h>
#include <scene/geometry/mesh.h>
#include <scene/geometry/squareplane.h>
#include <scene/materials/mattematerial.h>
#include <scene/lights/diffusearealight.h>

constexpr float screen_quad_pos[] = {
    1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
};

MyGL::MyGL(QWidget *parent)
    : GLWidget277(parent),
      sampler(new Sampler(100, 0)),
      integratorType(NAIVE_LIGHTING),
      recursionLimit(5),
      completeSFX(":/include/complete.wav")
{
    setFocusPolicy(Qt::ClickFocus);
    render_event_timer.setParent(this);
    connect(&render_event_timer, SIGNAL(timeout()), this, SLOT(onRenderUpdate()));
    move_rubberband = false;
    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    rubberBand->setGeometry(0,0,0,0);
    rubberBand->show();
    origin = QPoint(0, 0);
    rubberband_offset = QPoint(0, 0);
    something_rendered = false;
}

void MyGL::mousePressEvent(QMouseEvent *e)
{
    origin = e->pos();
    if (!rubberBand)
        rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    else rubberBand->hide();
    rubberBand->setGeometry(QRect(origin, QSize()));
    rubberBand->show();
}

void MyGL::mouseMoveEvent(QMouseEvent *e)
{
    rubberBand->setGeometry(QRect(origin, e->pos()).normalized());
}

void MyGL::mouseReleaseEvent(QMouseEvent *e)
{
    rubberband_offset = e->pos();
    reorderRect();
}

void MyGL::reorderRect() {
    int tmp_originx = (std::min(origin.x(), rubberband_offset.x()));
    int tmp_originy = (std::min(origin.y(), rubberband_offset.y()));

    int tmp_offsetx = (std::max(origin.x(), rubberband_offset.x()));
    int tmp_offsety = (std::max(origin.y(), rubberband_offset.y()));

    origin.setX(std::max(tmp_originx, 0));
    origin.setY(std::max(tmp_originy,0));
    rubberband_offset.setX(std::min(tmp_offsetx, (int)scene.camera.width));
    rubberband_offset.setY(std::min(tmp_offsety, (int)scene.camera.height));
}

MyGL::~MyGL()
{
    makeCurrent();

    vao.destroy();
}

void MyGL::initializeGL()
{
    // Create an OpenGL context
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    // Enable backface culling
    //    glEnable(GL_CULL_FACE);
    //    glCullFace(GL_BACK);
    //    glFrontFace(GL_CCW);

    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.5, 0.5, 0.5, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    vao.create();

    // Create and set up the diffuse shader
    prog_lambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat-color shader
    prog_flat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    vao.bind();

    // Create shader program for progressive render view
    prog_progressive.addShaderFromSourceFile(QOpenGLShader::Vertex  , ":/glsl/renderview.vert.glsl");
    prog_progressive.addShaderFromSourceFile(QOpenGLShader::Fragment,  ":/glsl/renderview.frag.glsl");
    prog_progressive_attribute_position = 0;
    prog_progressive_attribute_texcoord = 1;
    prog_progressive.bindAttributeLocation("position", prog_progressive_attribute_position);
    prog_progressive.bindAttributeLocation("uv", prog_progressive_attribute_texcoord);
    prog_progressive.link();
    // create full screen quad for progressive rendering
    glGenBuffers(1, &progressive_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, progressive_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screen_quad_pos), screen_quad_pos, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Test scene data initialization
    //scene.CreateTestScene();
    createCSGTestScene();
    ResizeToSceneCamera();
    gl_camera.CopyAttributes(scene.camera);
    rubberband_offset.setX(scene.camera.width);
    rubberband_offset.setY(scene.camera.height);
}

void MyGL::resizeGL(int w, int h)
{
    gl_camera = Camera(w, h);

    glm::mat4 viewproj = gl_camera.GetViewProj();

    // Upload the projection matrix
    prog_lambert.setViewProjMatrix(viewproj);
    prog_flat.setViewProjMatrix(viewproj);

    printGLErrorLog();
    gl_camera.CopyAttributes(scene.camera);
}

// This function is called by Qt any time your GL window is supposed to update
// For example, when the function updateGL is called, paintGL is called implicitly.
void MyGL::paintGL()
{
    if (progressive_render && (is_rendering || something_rendered))
    {
        glClear(GL_DEPTH_BUFFER_BIT);
        GLDrawProgressiveView();
    }
    else
    {    // Clear the screen so that we only see newly drawn images
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update the viewproj matrix
        prog_lambert.setViewProjMatrix(gl_camera.GetViewProj());
        prog_flat.setViewProjMatrix(gl_camera.GetViewProj());
        GLDrawScene();
    }
}

void MyGL::GLDrawScene()
{
    for(std::shared_ptr<Primitive> g : scene.primitives)
    {
        if(g->shape)
        {
            if(g->shape->drawMode() == GL_TRIANGLES)
            {
                if(g->areaLight != nullptr)
                {
                    prog_flat.setModelMatrix(g->shape->transform.T());
                    prog_flat.draw(*this, *g->shape);
                }
                else
                {
                    prog_lambert.setModelMatrix(g->shape->transform.T());
                    prog_lambert.draw(*this, *g->shape);
                }
            }
            else if(g->shape->drawMode() == GL_LINES)
            {
                prog_flat.setModelMatrix(g->shape->transform.T());
                prog_flat.draw(*this, *g->shape);
            }
        }
    }
    for(std::shared_ptr<Light> l : scene.lights)
    {
        DiffuseAreaLight* dal = dynamic_cast<DiffuseAreaLight*>(l.get());
        if(dal != nullptr)
        {
            prog_flat.setModelMatrix(dal->shape->transform.T());
            prog_flat.draw(*this, *dal->shape);
        }
    }
    prog_flat.setModelMatrix(glm::mat4(1.0f));
    prog_flat.draw(*this, scene.camera);
}

void MyGL::ResizeToSceneCamera()
{
    this->setFixedWidth(scene.camera.width);
    this->setFixedHeight(scene.camera.height);
    rubberband_offset.setX(scene.camera.width);
    rubberband_offset.setY(scene.camera.height);
    //    if(scene.camera.aspect <= 618/432)
    //    {
    //        this->setFixedWidth(432 / scene.camera.aspect);
    //        this->setFixedHeight(432);
    //    }
    //    else
    //    {
    //        this->setFixedWidth(618);
    //        this->setFixedHeight(618 * scene.camera.aspect);
    //    }
    //gl_camera = Camera(scene.camera);
}

void MyGL::keyPressEvent(QKeyEvent *e)
{
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }

    bool invalid_key = false;

    // http://doc.qt.io/qt-5/qt.html#Key-enum
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        gl_camera.RotateAboutUp(-amount);
    } else if (e->key() == Qt::Key_Left) {
        gl_camera.RotateAboutUp(amount);
    } else if (e->key() == Qt::Key_Up) {
        gl_camera.RotateAboutRight(-amount);
    } else if (e->key() == Qt::Key_Down) {
        gl_camera.RotateAboutRight(amount);
    } else if (e->key() == Qt::Key_1) {
        gl_camera.fovy += amount;
    } else if (e->key() == Qt::Key_2) {
        gl_camera.fovy -= amount;
    } else if (e->key() == Qt::Key_W) {
        gl_camera.TranslateAlongLook(amount);
    } else if (e->key() == Qt::Key_S) {
        gl_camera.TranslateAlongLook(-amount);
    } else if (e->key() == Qt::Key_D) {
        gl_camera.TranslateAlongRight(amount);
    } else if (e->key() == Qt::Key_A) {
        gl_camera.TranslateAlongRight(-amount);
    } else if (e->key() == Qt::Key_Q) {
        gl_camera.TranslateAlongUp(-amount);
    } else if (e->key() == Qt::Key_E) {
        gl_camera.TranslateAlongUp(amount);
    } else if (e->key() == Qt::Key_F) {
        gl_camera.CopyAttributes(scene.camera);
    } else if (e->key() == Qt::Key_G) {
        scene.camera = Camera(gl_camera);
        scene.camera.recreate();
    } else
    {
        invalid_key = true;
    }

    if (!invalid_key)
    {
        something_rendered = false;
        gl_camera.RecomputeAttributes();


        if(!is_rendering)
        {
            // If we moved the camera and we're not currently rendering,
            // then clean the pixels of our film.
            scene.film.cleanPixels();
        }

        update();  // Calls paintGL, among other things
    }
}

void MyGL::onRenderUpdate()
{
    if (!is_rendering)
        return;

    update();

    if (QThreadPool::globalInstance()->activeThreadCount() > 0)
    {
        return;
    }

    if (QThreadPool::globalInstance()->waitForDone())
    {
        completeRender();
    }
}

void MyGL::SceneLoadDialog()
{
    something_rendered = false;

    QString filepath = QFileDialog::getOpenFileName(0, QString("Load Scene"), QString("../scene_files"), tr("*.json"));
    if(filepath.length() == 0)
    {
        return;
    }

    QFile file(filepath);
    int i = filepath.length() - 1;
    while(QString::compare(filepath.at(i), QChar('/')) != 0)
    {
        i--;
    }
    QStringRef local_path = filepath.leftRef(i+1);
    //Reset all of our objects
    scene.Clear();
    //Load new objects based on the JSON file chosen.

    json_reader.LoadSceneFromFile(file, local_path, scene);
    gl_camera.CopyAttributes(scene.camera);
    ResizeToSceneCamera();
}

void MyGL::RenderScene()
{
    if (is_rendering) {return;}

    output_filepath = QFileDialog::getSaveFileName(0, QString("Save Image"), QString("../Sireesha_Renders"), tr("*.png"));
    if(output_filepath.length() == 0)
    {
        return;
    }
    // recreate progressive rendering texture
    if (progressive_texture)
    {
        progressive_texture->destroy();
        delete progressive_texture;
        progressive_texture = nullptr;
    }
    progressive_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    progressive_texture->setSize(scene.film.bounds.Diagonal().x, scene.film.bounds.Diagonal().y);
    progressive_texture->setFormat(QOpenGLTexture::TextureFormat::RGBA32F);
    progressive_texture->allocateStorage();
    // Get the bounds of the camera portion we are going to render
    // Ask the film for its bounds
    Bounds2i renderBounds = scene.film.bounds;
    // Change bounds to selected screen area if appropriate
    if(origin != rubberband_offset)
    {
        renderBounds = Bounds2i(Point2i(origin.x(), origin.y()), Point2i(rubberband_offset.x(), rubberband_offset.y()));
    }
    // Get the XY lengths of the film
    Vector2i renderExtent = renderBounds.Diagonal();
    // Compute the number of tiles needed in X and Y based on tileSize and width or height
    const int tileSize = 16;
    Point2i nTiles((renderExtent.x + tileSize - 1) / tileSize,
                   (renderExtent.y + tileSize - 1) / tileSize);

    renderTime = QDateTime::currentMSecsSinceEpoch();

    // For every tile, begin a render task:
    for(int X = 0; X < nTiles.x; X++)
    {
        for(int Y = 0; Y < nTiles.y; Y++)
        {
            int x0 = renderBounds.Min().x + X * tileSize;
            int x1 = std::min(x0 + tileSize, renderBounds.Max().x);
            int y0 = renderBounds.Min().y + Y * tileSize;
            int y1 = std::min(y0 + tileSize, renderBounds.Max().y);
            Bounds2i tileBounds(Point2i(x0, y0), Point2i(x1, y1));
            // Create a seed unique to the tile: pos.y * numberOfXTiles + pos.x
            int seed = y0 * nTiles.x + x0;

            Integrator* rt = nullptr;
            switch(integratorType)
            {
            case DIRECT_LIGHTING:
                rt = new DirectLightingIntegrator(tileBounds, &scene, sampler->Clone(seed), recursionLimit);
                break;
            case INDIRECT_LIGHTING:
                //TODO later
                break;
            case FULL_LIGHTING:
                rt = new FullLightingIntegrator(tileBounds, &scene, sampler->Clone(seed), recursionLimit);
                break;
            case NAIVE_LIGHTING:
                rt = new NaiveIntegrator(tileBounds, &scene, sampler->Clone(seed), recursionLimit);
                break;
            }
#define MULTITHREAD // Comment this line out to be able to debug with breakpoints.
#ifdef MULTITHREAD
            QThreadPool::globalInstance()->start(rt);
#else
            // Use this commented-out code to only render a tile with your desired pixel
            Point2i debugPixel(2,5);
            if(x0 < debugPixel.x && x1 >= debugPixel.x && y0 < debugPixel.y && y1 >= debugPixel.y)
            {
                rt->Render();
            }
            rt->Render();
            delete rt;
#endif //MULTITHREAD
        }
    }
    render_event_timer.start(500);
    is_rendering = true;
    something_rendered = true;
    emit sig_DisableGUI(true);
}

void MyGL::GLDrawProgressiveView()
{
    // if rendering, draw progressive rendering scene;
    prog_progressive.bind();

    Vector2i dims = scene.film.bounds.Diagonal();
    std::vector<glm::vec4> texdata(dims.x * dims.y);

    for (unsigned int x = 0; x < uint(dims.x); x++)
    {
        for (unsigned int y = 0; y < uint(dims.y); y++)
        {
            if (scene.film.IsPixelColorSet(Point2i(x, y)))
            {
                texdata[x + y * dims.x] = glm::vec4(scene.film.GetColor(Point2i(x, y)), 1.0f);
            }
            else
            {
                texdata[x + y * dims.x] = glm::vec4(0.0f);
            }
        }
    }

    progressive_texture->setData(QOpenGLTexture::PixelFormat::RGBA, QOpenGLTexture::PixelType::Float32, texdata.data());
    progressive_texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    progressive_texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // the initial texture is from the opengl frame buffer before start rendering
    progressive_texture->bind();

    glBindBuffer(GL_ARRAY_BUFFER, progressive_position_buffer);

    prog_progressive.enableAttributeArray(prog_progressive_attribute_position);
    glVertexAttribPointer(prog_progressive_attribute_position, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
    prog_progressive.enableAttributeArray(prog_progressive_attribute_texcoord);
    glVertexAttribPointer(prog_progressive_attribute_texcoord, 2, GL_FLOAT, GL_TRUE, 5*sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisable(GL_BLEND);
    progressive_texture->release();
}

void MyGL::completeRender()
{
    std::cout << "Milliseconds to render: " << QDateTime::currentMSecsSinceEpoch() - renderTime << std::endl;
    std::cout << "Seconds to render: " << (QDateTime::currentMSecsSinceEpoch() - renderTime) / 1000.f << std::endl;
    is_rendering = false;
    something_rendered = true;
    render_event_timer.stop();
    scene.film.WriteImage(output_filepath);
    completeSFX.play();
    emit sig_DisableGUI(false);
}

void MyGL::slot_SetNumSamplesSqrt(int i)
{
    sampler->samplesPerPixel = i * i;
}

void MyGL::slot_SetRecursionLimit(int n)
{
    recursionLimit = n;
}

void MyGL::slot_SetProgressiveRender(bool b)
{
    progressive_render = b;
}

void MyGL::slot_SetIntegratorType(int t)
{
    switch(t)
    {
    case 0:
        integratorType = NAIVE_LIGHTING;
        break;
    case 1:
        integratorType = DIRECT_LIGHTING;
        break;
    case 2:
        integratorType = INDIRECT_LIGHTING;
        break;
    case 3:
        integratorType = FULL_LIGHTING;
        break;
    }
}
void MyGL::createCSGTestScene()
{
    //Floor
    //Area light
    //Figure in front of light

    auto matteWhite = std::make_shared<MatteMaterial>(Color3f(0.725, 0.71, 0.68), 0, nullptr, nullptr);
    auto matteRed = std::make_shared<MatteMaterial>(Color3f(0.63, 0.065, 0.05), 0, nullptr, nullptr);
    auto matteGreen = std::make_shared<MatteMaterial>(Color3f(0.14, 0.45, 0.091), 0, nullptr, nullptr);

    // Floor, which is a large white plane
    auto floor = std::make_shared<SquarePlane>();
    floor->transform = Transform(Vector3f(0,-2.5,0), Vector3f(-90,0,0), Vector3f(10,10,1));
    auto floorPrim = std::make_shared<Primitive>(floor);
    floorPrim->material = matteWhite;
    floorPrim->name = QString("Floor");

    // Left wall, which is a large red plane
    auto leftWall = std::make_shared<SquarePlane>();
    leftWall->transform = Transform(Vector3f(5,2.5,0), Vector3f(0,-90,0), Vector3f(10,10,1));
    auto leftWallPrim = std::make_shared<Primitive>(leftWall);
    leftWallPrim->material = matteRed;
    leftWallPrim->name = QString("Left Wall");

    // Right wall, which is a large green plane
    auto rightWall = std::make_shared<SquarePlane>();
    rightWall->transform = Transform(Vector3f(-5,2.5,0), Vector3f(0,90,0), Vector3f(10,10,1));
    auto rightWallPrim = std::make_shared<Primitive>(rightWall);
    rightWallPrim->material = matteGreen;
    rightWallPrim->name = QString("Right Wall");

    // Back wall, which is a large white plane
    auto backWall = std::make_shared<SquarePlane>();
    backWall->transform = Transform(Vector3f(0,2.5,5), Vector3f(0,180,0), Vector3f(10,10,1));
    auto backWallPrim = std::make_shared<Primitive>(backWall);
    backWallPrim->material = matteWhite;
    backWallPrim->name = QString("Back Wall");

    // Ceiling, which is a large white plane
    auto ceiling = std::make_shared<SquarePlane>();
    ceiling->transform = Transform(Vector3f(0,7.5,0), Vector3f(90,0,0), Vector3f(10,10,1));
    auto ceilingPrim = std::make_shared<Primitive>(ceiling);
    ceilingPrim->material = matteWhite;
    ceilingPrim->name = QString("Ceiling");

    // Long cube
    auto longCube = std::make_shared<Cube>();
    longCube->transform = Transform(Vector3f(2, 0, 3), Vector3f(0, 27.5, 0), Vector3f(3, 6, 3));
    auto longCubePrim = std::make_shared<Primitive>(longCube);
    longCubePrim->material = matteWhite;
    longCubePrim->name = QString("Long Cube");

    // Short cube
    auto shortCube = std::make_shared<Cube>();
    shortCube->transform = Transform(Vector3f(-2, -1, 0.75), Vector3f(0, -17.5, 0), Vector3f(3, 3, 3));
    auto shortCubePrim = std::make_shared<Primitive>(shortCube);
    shortCubePrim->material = matteWhite;
    shortCubePrim->name = QString("Short Cube");

    // Light source, which is a diffuse area light with a large plane as its shape
    auto lightSquare = std::make_shared<SquarePlane>();
    lightSquare->transform = Transform(Vector3f(0,7.45f,0), Vector3f(90,0,0), Vector3f(3, 3, 1));
    auto lightSource = std::make_shared<DiffuseAreaLight>(lightSquare->transform, Color3f(17,12,4) * 2.f, lightSquare);
    auto lightPrim = std::make_shared<Primitive>(lightSquare, nullptr, lightSource);
    lightPrim->name = QString("Light Source");

    //CSG
    auto longCube1 = std::make_shared<Cube>();
    longCube1->transform = Transform(Vector3f(0, 0, 0), Vector3f(90, 0, -27.5), Vector3f(3, 6, 3));
    auto csgCube1 = std::make_shared<CSGNode>();
    csgCube1->type = GEOM; //Geometry
    csgCube1->geom =longCube1.get();

    auto longCube2 = std::make_shared<Cube>();
    longCube2->transform = Transform(Vector3f(0, 0, 0), Vector3f(90, 0, 62.5), Vector3f(3, 6, 3));
    auto csgCube2 = std::make_shared<CSGNode>();
    csgCube2->type = GEOM; //Geometry
    csgCube2->geom =longCube2.get();

    // Large sphere:
    auto sphere = std::make_shared<Sphere>();
    sphere->transform = Transform(Vector3f(0, 0, 0), Vector3f(0, 0, 0), Vector3f(3, 3, 3));
    auto csgSphere = std::make_shared<CSGNode>();
    csgSphere->type = GEOM; //Geometry
    csgSphere->geom = sphere.get();

    auto longCube3 = std::make_shared<Cube>();
    longCube3->transform = Transform(Vector3f(0, 0, 0), Vector3f(0, 27.5, 0), Vector3f(3, 6, 3));
    auto csgCube3 = std::make_shared<CSGNode>();
    csgCube3->type = GEOM; //Geometry
    csgCube3->geom =longCube3.get();

    auto csgLeftDiffer = std::make_shared<CSGNode>();
    csgLeftDiffer->type = D; //Difference
    csgLeftDiffer->left = csgCube2.get();
    csgLeftDiffer->right = csgCube3.get();


    auto csgRightUnion = std::make_shared<CSGNode>();
    csgRightUnion->type = U; //Union
    csgRightUnion->left = csgCube2.get();
    csgRightUnion->right = csgCube1.get();

    auto root = std::make_shared<CSGNode>();
    root->type = N; //Intersection
    root->left= csgLeftDiffer.get();
    root->right = csgSphere.get();

    auto csgPrim = std::make_shared<Primitive>(root);
    csgPrim->material = matteWhite;
    csgPrim->name = QString("csg Primitive");

    scene.primitives.append(floorPrim);
    scene.primitives.append(lightPrim);
    scene.primitives.append(leftWallPrim);
    scene.primitives.append(rightWallPrim);
    scene.primitives.append(backWallPrim);
    scene.primitives.append(ceilingPrim);
    scene.primitives.append(csgPrim);

    scene.lights.append(lightSource);

    for(std::shared_ptr<Primitive> p : scene.primitives)
    {
        p->shape->create();
    }

    scene.camera = Camera(400, 400, Point3f(0, 5.5, -30), Point3f(0,2.5,0), Vector3f(0,1,0));
    scene.camera.near_clip = 0.1f;
    scene.camera.far_clip = 100.0f;
    scene.camera.fovy = 19.5f;
    scene.camera.create();
    scene.camera.RecomputeAttributes();
    scene.film = Film(400, 400);
}
