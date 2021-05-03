#include "camera.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include "glm/glm.hpp"
#include "utils/misc.h"
#include <glm/gtc/matrix_transform.hpp>

using std::cout;
using std::endl;
using std::max;
using std::min;
using std::ifstream;
using std::ofstream;
using utils::radians;
using utils::degrees;
using utils::clamp;

namespace fluid_camera {

// copied from proj4
void Camera::configure(const CameraInfo &info, size_t screenW, size_t screenH) {
  this->screenW = screenW;
  this->screenH = screenH;
  nClip = info.nClip;
  fClip = info.fClip;
  hFov = info.hFov;
  vFov = info.vFov;

  double ar1 = tan(radians(hFov) / 2) / tan(radians(vFov) / 2);
  ar = static_cast<double>(screenW) / screenH;
  if (ar1 < ar) {
    // hFov is too small
    hFov = 2 * degrees(atan(tan(radians(vFov) / 2) * ar));
  } else if (ar1 > ar) {
    // vFov is too small
    vFov = 2 * degrees(atan(tan(radians(hFov) / 2) / ar));
  }
  screenDist = ((double)screenH) / (2.0 * tan(radians(vFov) / 2));
}

void Camera::place(const glm::vec3 &targetPos, const double phi,
                   const double theta, const double r, const double minR,
                   const double maxR) {
  double r_ = min(max(r, minR), maxR);
  double phi_ = (sin(phi) == 0) ? (phi + EPS_F) : phi;
  this->targetPos = targetPos;
  this->phi = phi_;
  this->theta = theta;
  this->r = r_;
  this->minR = minR;
  this->maxR = maxR;
  compute_position();
}

void Camera::copy_placement(const Camera &other) {
  pos = other.pos;
  targetPos = other.targetPos;
  phi = other.phi;
  theta = other.theta;
  minR = other.minR;
  maxR = other.maxR;
  c2w = other.c2w;
}

void Camera::set_screen_size(const size_t screenW, const size_t screenH) {
  this->screenW = screenW;
  this->screenH = screenH;
  ar = 1.0 * screenW / screenH;
  hFov = 2 * degrees(atan(((double)screenW) / (2 * screenDist)));
  vFov = 2 * degrees(atan(((double)screenH) / (2 * screenDist)));
}

void Camera::move_by(const double dx, const double dy, const double d) {
  const double scaleFactor = d / screenDist;
  const glm::vec3  &displacement =
      c2w[0] * (float)(dx * scaleFactor) + c2w[1] * (float)(dy * scaleFactor);
  pos += displacement;
  targetPos += displacement;
}

void Camera::move_forward(const double dist) {
  double newR = min(max(r - dist, minR), maxR);
  pos = targetPos + ((pos - targetPos) * (float)(newR / r));
  r = newR;
}

void Camera::rotate_by(const double dPhi, const double dTheta) {
  phi = clamp(phi + dPhi, 0.0, (double)PI);
  theta += dTheta;
  compute_position();
}

void Camera::compute_position() {
  double sinPhi = sin(phi);
  if (sinPhi == 0) {
    phi += EPS_F;
    sinPhi = sin(phi);
  }
  const glm::vec3 dirToCamera(r * sinPhi * sin(theta), r * cos(phi),
                             r * sinPhi * cos(theta));
  pos = targetPos + dirToCamera;
  glm::vec3 upVec(0, sinPhi > 0 ? 1 : -1, 0);
  glm::vec3 screenXDir = cross(upVec, dirToCamera);
  screenXDir = glm::normalize(screenXDir);
  glm::vec3 screenYDir = cross(dirToCamera, screenXDir);
  screenYDir = glm::normalize(screenYDir);

  c2w[0] = screenXDir;
  c2w[1] = screenYDir;
  glm::vec3 dir_to_camera_norm = glm::normalize(dirToCamera);
  c2w[2] = dir_to_camera_norm; // camera's view direction is the
                               // opposite of of dirToCamera, so
                               // directly using dirToCamera as
                               // column 2 of the matrix takes [0 0 -1]
                               // to the world space view direction
}

} 