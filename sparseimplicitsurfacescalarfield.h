/*
Copyright (c) 2015 Ryan L. Guy

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgement in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#pragma once

#include <stdio.h>
#include <iostream>

#include "glm/glm.hpp"
#include "sparsearray3d.h"
#include "grid3d.h"
#include "aabb.h"

class SparseImplicitSurfaceScalarField
{
public:
    SparseImplicitSurfaceScalarField();
    SparseImplicitSurfaceScalarField(int i, int j, int k, double dx);
    ~SparseImplicitSurfaceScalarField();

    void getGridDimensions(int *i, int *j, int *k);
    double getCellSize();

    void clear();
    void reserve(int n);
    void setPointRadius(double r);
    void addPoint(glm::vec3 pos, double radius);
    void addPoint(glm::vec3 pos);
    void addPointValue(glm::vec3 pos, double radius, double value);
    void addPointValue(glm::vec3 pos, double value);
    void addCuboid(glm::vec3 pos, double w, double h, double d);
    void setSurfaceThreshold(double t) { _surfaceThreshold = t; }
    double getSurfaceThreshold() { return _surfaceThreshold; }
    void setSolidCells(std::vector<GridIndex> &solidCells);
    void getScalarField(SparseArray3d<float> &field);
    void setTricubicWeighting();
    void setTrilinearWeighting();

private:

    inline double _hatFunc(double r) {
        if (r >= 0.0 && r <= 1.0) {
            return 1 - r;
        } else if (r >= -1.0 && r <= 0.0) {
            return 1 + r;
        }

        return 0.0;
    }

    double _evaluateTricubicFieldFunctionForRadiusSquared(double rsq);
    double _evaluateTrilinearFieldFunction(glm::vec3 v);

    int M_SOLID = 2;
    int WEIGHT_TRICUBIC = 0;
    int WEIGHT_TRILINEAR = 1;
    int _weightType = 0;

    int _isize = 0;
    int _jsize = 0;
    int _ksize = 0;
    double _dx = 0.0;

    double _radius = 0.0;
    double _invRadius = 1.0;
    double _coef1 = 0.0;
    double _coef2 = 0.0;
    double _coef3 = 0.0;

    double _surfaceThreshold = 0.5;

    SparseArray3d<float> _field;
    SparseArray3d<bool> _isVertexSolid;
};

