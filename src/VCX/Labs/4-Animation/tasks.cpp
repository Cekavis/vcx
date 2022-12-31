#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <spdlog/spdlog.h>
#include <iostream>
#include "Labs/4-Animation/tasks.h"
#include "IKSystem.h"
#include "CustomFunc.inl"


namespace VCX::Labs::Animation {
    void ForwardKinematics(IKSystem & ik, int StartIndex) {
        if (StartIndex == 0) {
            ik.JointGlobalRotation[0] = ik.JointLocalRotation[0];
            ik.JointGlobalPosition[0] = ik.JointLocalOffset[0];
            StartIndex                = 1;
        }
        
        for (int i = StartIndex; i < ik.JointLocalOffset.size(); i++) {
            ik.JointGlobalRotation[i] = ik.JointGlobalRotation[i - 1] * ik.JointLocalRotation[i];
            ik.JointGlobalPosition[i] = ik.JointGlobalPosition[i - 1] + ik.JointGlobalRotation[i - 1] * ik.JointLocalOffset[i];
        }
    }

    void InverseKinematicsCCD(IKSystem & ik, const glm::vec3 & EndPosition, int maxCCDIKIteration, float eps) {
        ForwardKinematics(ik, 0);
        // These functions will be useful: glm::normalize, glm::rotation, glm::quat * glm::quat
        int cnt = 0;
        for (int CCDIKIteration = 0; CCDIKIteration < maxCCDIKIteration && glm::l2Norm(ik.EndEffectorPosition() - EndPosition) > eps; CCDIKIteration++) {
            glm::vec3 endPos = ik.JointGlobalPosition.back();
            for (int i = ik.JointLocalOffset.size() - 1; i > 0; i--) {
                glm::vec3 jointPos = ik.JointGlobalPosition[i - 1];
                glm::vec3 offset = endPos - jointPos;
                glm::quat rotation = glm::rotation(glm::normalize(offset), glm::normalize(EndPosition - jointPos));
                ik.JointLocalRotation[i - 1] *= rotation;
                endPos = jointPos + rotation * offset;
            }
            ForwardKinematics(ik, 0);
            ++cnt;
        }
        std::cerr << "CCD IK Iteration: " << cnt << std::endl;
    }

    void InverseKinematicsFABR(IKSystem & ik, const glm::vec3 & EndPosition, int maxFABRIKIteration, float eps) {
        ForwardKinematics(ik, 0);
        int nJoints = ik.NumJoints();
        std::vector<glm::vec3> backward_positions(nJoints, glm::vec3(0, 0, 0)), forward_positions(nJoints, glm::vec3(0, 0, 0));
        int cnt = 0;
        for (int IKIteration = 0; IKIteration < maxFABRIKIteration && glm::l2Norm(ik.EndEffectorPosition() - EndPosition) > eps; IKIteration++) {
            // task: fabr ik
            // backward update
            glm::vec3 next_position         = EndPosition;
            backward_positions[nJoints - 1] = EndPosition;

            for (int i = nJoints - 2; i >= 0; i--) {
                glm::vec3 offset = next_position - ik.JointGlobalPosition[i];
                next_position -= glm::normalize(offset) * glm::length(ik.JointLocalOffset[i + 1]);
                backward_positions[i] = next_position;
            }

            // forward update
            glm::vec3 now_position = ik.JointGlobalPosition[0];
            forward_positions[0] = ik.JointGlobalPosition[0];
            for (int i = 0; i < nJoints - 1; i++) {
                glm::vec3 offset = backward_positions[i + 1] - now_position;
                now_position += glm::normalize(offset) * glm::length(ik.JointLocalOffset[i + 1]);
                forward_positions[i + 1] = now_position;
            }
            ik.JointGlobalPosition = forward_positions; // copy forward positions to joint_positions
            ++cnt;
        }
        std::cerr << "FABRIK Iteration: " << cnt << std::endl;

        // Compute joint rotation by position here.
        for (int i = 0; i < nJoints - 1; i++) {
            ik.JointGlobalRotation[i] = glm::rotation(glm::normalize(ik.JointLocalOffset[i + 1]), glm::normalize(ik.JointGlobalPosition[i + 1] - ik.JointGlobalPosition[i]));
        }
        ik.JointLocalRotation[0] = ik.JointGlobalRotation[0];
        for (int i = 1; i < nJoints - 1; i++) {
            ik.JointLocalRotation[i] = glm::inverse(ik.JointGlobalRotation[i - 1]) * ik.JointGlobalRotation[i];
        }
        ForwardKinematics(ik, 0);
    }

    IKSystem::Vec3ArrPtr IKSystem::BuildCustomTargetPosition() {
        int nums = 30;
        using Vec3Arr = std::vector<glm::vec3>;
        std::shared_ptr<Vec3Arr> custom(new Vec3Arr(nums * 19));
        int index = 0;

        auto horizontal = [&](float offsetX, float offsetY) {
            for (int i = 0; i < nums; i++) {
                float x_val = offsetX - .5 * i / nums;
                float y_val = offsetY;
                (*custom)[index++] = glm::vec3(x_val, 0.0f, y_val);
            }
        };
        auto vertical = [&](float offsetX, float offsetY) {
            for (int i = 0; i < nums; i++) {
                float x_val = offsetX;
                float y_val = offsetY - .5 * i / nums;
                (*custom)[index++] = glm::vec3(x_val, 0.0f, y_val);
            }
        };
        auto digitToPoints = [&](int d, float offset) {
            if (d == 1) {
                vertical(offset, 0);
                vertical(offset, .5);
            }
            if (d == 4) {
                vertical(offset, 0);
                vertical(offset, .5);
                horizontal(offset, 0);
                vertical(offset - .5, 0);
            }
            if (d == 5) {
                horizontal(offset, 0);
                horizontal(offset, .5);
                horizontal(offset, -.5);
                vertical(offset, .5);
                vertical(offset - .5, 0);
            }
        };

        digitToPoints(1, -1.7);
        digitToPoints(1, -1.0);
        digitToPoints(4, -0.3);
        digitToPoints(5, 0.4);
        digitToPoints(1, 1.1);
        digitToPoints(4, 1.8);
        return custom;
    }

    void AdvanceMassSpringSystem(MassSpringSystem & system, float const dt) {
        // your code here: rewrite following code
        int const steps = 10;
        float const ddt = dt / steps;
        for (std::size_t s = 0; s < steps; s++) {
            int n = system.Positions.size();
            Eigen::SparseMatrix<float> M(3*n, 3*n);
            std::vector<Eigen::Triplet<float>> coefficients;
            Eigen::VectorXf b = Eigen::VectorXf::Zero(3*n);

            auto addCoefficient = [&](int i, int j, glm::vec3 a) {
                coefficients.push_back(Eigen::Triplet<float>(3*i, 3*j, a.x));
                coefficients.push_back(Eigen::Triplet<float>(3*i+1, 3*j+1, a.y));
                coefficients.push_back(Eigen::Triplet<float>(3*i+2, 3*j+2, a.z));
            };

            auto addB = [&](int i, glm::vec3 a) {
                b[3*i] += a.x;
                b[3*i+1] += a.y;
                b[3*i+2] += a.z;
            };

            for (int i = 0; i < n; i++) {
                addCoefficient(i, i, glm::vec3(1));
                addB(i, system.Velocities[i] + glm::vec3(0, -ddt * system.Gravity, 0));
            }
            for (auto const spring : system.Springs) {
                auto const p0 = spring.AdjIdx.first;
                auto const p1 = spring.AdjIdx.second;
                glm::vec3 const x01 = system.Positions[p1] - system.Positions[p0];
                glm::vec3 const v01 = system.Velocities[p1] - system.Velocities[p0];
                glm::vec3 const e01 = glm::normalize(x01);
                glm::vec3 f = (system.Stiffness * (glm::length(x01) - spring.RestLength) + system.Damping * glm::dot(v01, e01)) * e01;
                addB(p0, f * ddt / system.Mass);
                addB(p1, -f * ddt / system.Mass);
                
                addCoefficient(p0, p0, glm::vec3(-ddt * ddt / system.Mass));
                addCoefficient(p0, p1, glm::vec3(ddt * ddt / system.Mass));
                addCoefficient(p1, p1, glm::vec3(-ddt * ddt / system.Mass));
                addCoefficient(p1, p0, glm::vec3(ddt * ddt / system.Mass));
            }

            M.setFromTriplets(coefficients.begin(), coefficients.end());
            auto solver = Eigen::SimplicialLLT<Eigen::SparseMatrix<float>>(M);
            Eigen::VectorXf x = solver.solve(b);
            for (int i = 0; i < n; i++) {
                if (system.Fixed[i]) continue;
                system.Velocities[i] = glm::vec3(x[3*i], x[3*i+1], x[3*i+2]);
                system.Positions[i] += system.Velocities[i] * ddt;
            }
        }
    }
}
