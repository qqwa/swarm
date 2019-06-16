#include "wind.h"

Wind::Wind() : m_transform({0, 0, 0}) {
    auto s = 10;
    m_transform.SetScale({s, s, s});
    arrow_color = {1.0, 1.0, 1.0};
    arrow_mesh = util::loadMesh("res/arrow.obj");
    arrow_shader = util::getShader("res/shader/default");
}

void Wind::update(float delta) {
    // do nothing for now
}

void Wind::render(Camera &camera) {
    glUseProgram(arrow_shader);
    int viewLocation = glGetUniformLocation(arrow_shader, "view");
    auto view = camera.GetTransformWithoutTranslation();
    view[3][0] = 50;
    view[3][1] = 50;
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    int projectionLocation = glGetUniformLocation(arrow_shader, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE,
                       glm::value_ptr(glm::ortho(0.0f, 800.0f, 0.0f, 600.0f,
                                                 -100.0f, 100.0f)));
    int colorLocation = glGetUniformLocation(arrow_shader, "color");
    glUniform3fv(colorLocation, 1, glm::value_ptr(arrow_color));

    int modelLocation = glGetUniformLocation(arrow_shader, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE,
                       glm::value_ptr(m_transform.GetMatrix()));
    glBindVertexArray(arrow_mesh.vao);
    glDrawElements(GL_TRIANGLES, arrow_mesh.indicesCount, GL_UNSIGNED_INT, 0);
}