#include "wind.h"
#include "config.h"

Wind::Wind()
    : m_direction(glm::angleAxis(glm::radians(80.0f), glm::vec3{0, 0, -1})) {
    arrow_color = {1.0, 1.0, 1.0};
    arrow_mesh = util::loadMesh("res/arrow.obj");
    arrow_shader = util::getShader("res/shader/default");
}

void Wind::update(int tick) {
    // do nothing for now
    m_direction = config->get_wind_dir(tick);
    m_strength = config->get_wind_strength(tick);
}

void Wind::render(Camera &camera) {
    glUseProgram(arrow_shader);
    int viewLocation = glGetUniformLocation(arrow_shader, "view");
    auto view = camera.GetTransformWithoutTranslation();
    view[3][0] = 75;
    view[3][1] = 75;
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    int projectionLocation = glGetUniformLocation(arrow_shader, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE,
                       glm::value_ptr(glm::ortho(0.0f, 800.0f, 0.0f, 600.0f,
                                                 -100.0f, 100.0f)));
    int colorLocation = glGetUniformLocation(arrow_shader, "color");
    glUniform3fv(colorLocation, 1, glm::value_ptr(arrow_color));

    auto s = 5;
    auto transform =
        Transform({0, 0, 0}, m_direction, {s, s * (m_strength / 5), s});
    int modelLocation = glGetUniformLocation(arrow_shader, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE,
                       glm::value_ptr(transform.GetMatrix()));
    glBindVertexArray(arrow_mesh.vao);
    glDrawElements(GL_TRIANGLES, arrow_mesh.indicesCount, GL_UNSIGNED_INT, 0);
}

glm::vec3 Wind::get_force() const {
    // normalized direction times strength
    return {0, 0, 0};
}
