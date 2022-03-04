#include "SceneRenderer.h"

SceneRenderer::SceneRenderer() {}

SceneRenderer::~SceneRenderer() {}

void SceneRenderer::RenderLayer()
{
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SceneRenderer::Update(float dt)
{
    RenderLayer();
}
