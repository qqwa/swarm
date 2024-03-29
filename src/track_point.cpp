#include "track_point.h"
#include "config.h"

TrackPoint::TrackPoint() : m_transform({0.0, 0.0, 0.0}) {
    track_point_color = {0.0, 0.7, 0.3};
    track_point_mesh = util::loadMesh("res/sphere.obj");
    track_point_shader = util::getShader("res/shader/default");
    m_transform.SetScale(glm::vec3{config->debug_sphere_size,
                                   config->debug_sphere_size,
                                   config->debug_sphere_size});
}

void TrackPoint::update(int tick) {
    auto pos = config->get_track_point(tick);
    m_transform.SetPosition(pos);
}

void TrackPoint::render(Camera &camera) {
    glUseProgram(track_point_shader);
    int viewLocation = glGetUniformLocation(track_point_shader, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE,
                       glm::value_ptr(camera.GetTransform()));
    int projectionLocation =
        glGetUniformLocation(track_point_shader, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE,
                       glm::value_ptr(camera.GetProjection()));
    int colorLocation = glGetUniformLocation(track_point_shader, "color");
    glUniform3fv(colorLocation, 1, glm::value_ptr(track_point_color));

    int modelLocation = glGetUniformLocation(track_point_shader, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE,
                       glm::value_ptr(m_transform.GetMatrix()));
    glBindVertexArray(track_point_mesh.vao);
    glDrawElements(GL_TRIANGLES, track_point_mesh.indicesCount, GL_UNSIGNED_INT,
                   0);
}

glm::vec3 TrackPoint::get_pos() const { return m_transform.GetPosition(); }