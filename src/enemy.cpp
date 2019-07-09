#include "enemy.h"
#include "config.h"

Enemy::Enemy() : m_transform({0.0, 0.0, 0.0}) {
    enemy_color = {1.0, 0.0, 0.0};
    enemy_mesh = util::loadMesh("res/sphere.obj");
    enemy_shader = util::getShader("res/shader/default");
}

void Enemy::update(int tick) {
    auto pos = config->get_enemy_pos(tick);
    auto scale = config->get_enemy_scale(tick);
    m_transform.SetPosition(pos);
    m_transform.SetScale(glm::vec3{scale});
}

void Enemy::render(Camera &camera) {
    glUseProgram(enemy_shader);
    int viewLocation = glGetUniformLocation(enemy_shader, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE,
                       glm::value_ptr(camera.GetTransform()));
    int projectionLocation = glGetUniformLocation(enemy_shader, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE,
                       glm::value_ptr(camera.GetProjection()));
    int colorLocation = glGetUniformLocation(enemy_shader, "color");
    glUniform3fv(colorLocation, 1, glm::value_ptr(enemy_color));

    int modelLocation = glGetUniformLocation(enemy_shader, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE,
                       glm::value_ptr(m_transform.GetMatrix()));
    glBindVertexArray(enemy_mesh.vao);
    glDrawElements(GL_TRIANGLES, enemy_mesh.indicesCount, GL_UNSIGNED_INT, 0);
}

glm::vec3 Enemy::get_pos() const { return m_transform.GetPosition(); }