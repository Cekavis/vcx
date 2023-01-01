#include "Labs/5-Visualization/tasks.h"
#include <iostream>
#include <tuple>

#include <numbers>

using VCX::Labs::Common::ImageRGB;
namespace VCX::Labs::Visualization {

    struct CoordinateStates {
        std::string const captions[7] = {"cylinders", "displacement", "weight", "horsepower", "acceleration", "mileage", "year"};
        glm::vec4 const defaultBarColor = glm::vec4(0.8f, 0.8f, 0.8f, 0.5f);
        glm::vec4 const selectedBarColor = glm::vec4(0.67f, 0.39f, 0.43f, 0.8f);
        glm::vec4 const defaultLineColor = glm::vec4(0.49f, 0.56f, 0.69f, 1);
        glm::vec4 const selectedLineColor = glm::vec4(0.55f, 0.22f, 0.24f, 1);
        std::vector<std::vector<float>> cars;
        float minValue[7], maxValue[7];
        glm::vec4 barColor[7];
        std::vector<glm::vec4> lineColor;
        glm::vec2 mousePos;

        CoordinateStates(std::vector<Car> const & data) {
            std::fill(minValue, minValue + 7, std::numeric_limits<float>::max());
            std::fill(maxValue, maxValue + 7, std::numeric_limits<float>::min());
            std::fill(barColor, barColor + 7, defaultBarColor);
            lineColor.resize(data.size(), defaultLineColor);
            mousePos = glm::vec2(0);

            for (auto const & car : data) {
                cars.push_back({(float)car.cylinders, car.displacement, car.weight, car.horsepower, car.acceleration, car.mileage, (float)car.year});
                for (int i = 0; i < 7; ++i) {
                    minValue[i] = std::min(minValue[i], cars.back()[i]);
                    maxValue[i] = std::max(maxValue[i], cars.back()[i]);
                }
            }
            for (int i = 0; i < 7; ++i) {
                std::tie(minValue[i], maxValue[i]) = std::make_pair(
                    floor(minValue[i] - (maxValue[i] - minValue[i]) * 0.1f),
                    ceil(maxValue[i] + (maxValue[i] - minValue[i]) * 0.1f)
                );
                for (auto & car : cars) {
                    car[i] = (car[i] - minValue[i]) / (maxValue[i] - minValue[i]);
                }
            }
        }
        void Update(InteractProxy const & proxy) {
            std::fill(barColor, barColor + 7, defaultBarColor);
            std::fill(lineColor.begin(), lineColor.end(), defaultLineColor);
            if (proxy.IsHovering()) mousePos = proxy.MousePos();
            int x = floor(mousePos.x * 7 / 1.0f);
            barColor[x] = selectedBarColor;
            for (int i = 0; i < cars.size(); ++i) {
                lineColor[i] = cars[i][x] * selectedLineColor + (1 - cars[i][x]) * defaultLineColor;
            }
        }
    };

    bool PaintParallelCoordinates(Common::ImageRGB & input, InteractProxy const & proxy, std::vector<Car> const & data, bool force) {
        float const innerLineWidth = 2;
        float const outerLineWidth = 17, outerLineWidthRatio = outerLineWidth / input.GetSizeX();
        float const whiteLineWidth = 2, whiteLineWidthRatio = whiteLineWidth / input.GetSizeX();

        static CoordinateStates states(data);
        states.Update(proxy);
        SetBackGround(input, glm::vec4(1));
        for (int c = 0; c < (int)data.size(); ++c) {
            for (int i = 0; i < 6; ++i) {
                DrawLine(input, states.lineColor[c], glm::vec2(1.0f / 7 * (i + 0.5f), 0.93f - 0.83f * states.cars[c][i]), glm::vec2(1.0f / 7 * (i + 1.5f), 0.93f - 0.83f * states.cars[c][i + 1]), 1);
            }
        }
        for (int i = 0; i < 7; ++i) {
            PrintText(input, glm::vec4(0, 0, 0, 1), glm::vec2(1.0f / 7 * (i + 0.5f), 0.04f), 0.02f, states.captions[i]);
            PrintText(input, glm::vec4(0, 0, 0, 1), glm::vec2(1.0f / 7 * (i + 0.5f), 0.07f), 0.02f, std::to_string((int)states.maxValue[i]));
            PrintText(input, glm::vec4(0, 0, 0, 1), glm::vec2(1.0f / 7 * (i + 0.5f), 0.96f), 0.02f, std::to_string((int)states.minValue[i]));
            DrawLine(input, states.barColor[i], glm::vec2(1.0f / 7 * (i + 0.5f) + outerLineWidthRatio / 2, 0.1f), glm::vec2(1.0f / 7 * (i + 0.5f) + outerLineWidthRatio / 2, 0.93f), outerLineWidth);
            DrawLine(input, glm::vec4(0.6f, 0.6f, 0.6f, 1), glm::vec2(1.0f / 7 * (i + 0.5f), 0.1f), glm::vec2(1.0f / 7 * (i + 0.5f), 0.93f), innerLineWidth);
            DrawLine(input, glm::vec4(1), glm::vec2(1.0f / 7 * (i + 0.5f) + outerLineWidthRatio / 2 + whiteLineWidthRatio, 0.1f), glm::vec2(1.0f / 7 * (i + 0.5f) + outerLineWidthRatio / 2 + whiteLineWidthRatio, 0.93f), whiteLineWidth);
            DrawLine(input, glm::vec4(1), glm::vec2(1.0f / 7 * (i + 0.5f) - outerLineWidthRatio / 2, 0.1f), glm::vec2(1.0f / 7 * (i + 0.5f) - outerLineWidthRatio / 2, 0.93f), whiteLineWidth);
        }
        return true;
    }

    void LIC(ImageRGB & output, Common::ImageRGB const & noise, VectorField2D const & field, int const & step) {
        // your code here
    }
}; // namespace VCX::Labs::Visualization