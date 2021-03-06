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
#include <vector>
#include <thread>
#include <unordered_map>
#include <assert.h>

#include <Eigen\Core>
#include <Eigen\SparseCore>
#include <Eigen\IterativeLinearSolvers>
#include <gl\glew.h>
#include <SDL_opengl.h>
#include <gl\glu.h>

#include "stopwatch.h"
#include "macvelocityfield.h"
#include "array3d.h"
#include "sparsearray3d.h"
#include "grid3d.h"
#include "surfacefield.h"
#include "levelsetfield.h"
#include "implicitsurfacescalarfield.h"
#include "sparseimplicitsurfacescalarfield.h"
#include "polygonizer3d.h"
#include "sparsepolygonizer3d.h"
#include "triangleMesh.h"
#include "logfile.h"
#include "collision.h"
#include "aabb.h"
#include "levelset.h"
#include "fluidsimulationsavestate.h"
#include "fluidsource.h"
#include "sphericalfluidsource.h"
#include "cuboidfluidsource.h"
#include "turbulencefield.h"
#include "fluidbrickgrid.h"
#include "glm/glm.hpp"

struct MarkerParticle {
    glm::vec3 position = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 velocity = glm::vec3(0.0, 0.0, 0.0);

    MarkerParticle() : position(0.0, 0.0, 0.0), 
                        velocity(0.0, 0.0, 0.0) {}

    MarkerParticle(glm::vec3 p) : position(p),
                                  velocity(0.0, 0.0, 0.0) {}
    MarkerParticle(glm::vec3 p, glm::vec3 v) : 
                                  position(p),
                                  velocity(v) {}

    MarkerParticle(double x, double y, double z) : 
                                  position(x, y, z),
                                  velocity(0.0, 0.0, 0.0) {}
};

struct DiffuseParticle {
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
    int type;

    DiffuseParticle() : position(0.0, 0.0, 0.0),
                        velocity(0.0, 0.0, 0.0),
                        lifetime(0.0),
                        type(-1) {}

    DiffuseParticle(glm::vec3 p, glm::vec3 v, float time) : 
                        position(p),
                        velocity(v),
                        lifetime(time),
                        type(-1) {}
};

class FluidSimulation
{
public:
    FluidSimulation();
    FluidSimulation(int x_voxels, int y_voxels, int z_voxels, double cell_size);
    FluidSimulation(FluidSimulationSaveState &state);
    ~FluidSimulation();

    void run();
    void pause();
    void update(double dt);
    void saveState();
    void saveState(std::string filename);
    int getCurrentFrame();
    bool isCurrentFrameFinished();

    double getCellSize();
    void getGridDimensions(int *i, int *j, int *k);
    void getSimulationDimensions(double *w, double *h, double *d);
    double getSimulationWidth();
    double getSimulationHeight();
    double getSimulationDepth();
    double getDensity();
    void setDensity(double p);
    int getMaterial(int i, int j, int k);
    void setMarkerParticleScale(double s);
    void setSurfaceSubdivisionLevel(unsigned int n);

    void enableSurfaceMeshOutput();
    void disableSurfaceMeshOutput();
    void enableDiffuseMaterialOutput();
    void disableDiffuseMaterialOutput();
    void enableBrickOutput();
    void enableBrickOutput(double width, double height, double depth);
    void disableBrickOutput();

    void addBodyForce(double fx, double fy, double fz);
    void addBodyForce(glm::vec3 f);
    void setBodyForce(double fx, double fy, double fz);
    void setBodyForce(glm::vec3 f);
    void addImplicitFluidPoint(double x, double y, double z, double r);
    void addImplicitFluidPoint(glm::vec3 p, double radius);
    void addFluidCuboid(double x, double y, double z, double w, double h, double d);
    void addFluidCuboid(glm::vec3 p, double width, double height, double depth);
    void addFluidCuboid(glm::vec3 p1, glm::vec3 p2);

    bool addFluidMesh(std::string OBJFilename);
    bool addFluidMesh(std::string OBJFilename, glm::vec3 offset);
    bool addFluidMesh(std::string OBJFilename, double scale);
    bool addFluidMesh(std::string OBJFilename, glm::vec3 offset, double scale);

    SphericalFluidSource *addSphericalFluidSource(glm::vec3 pos, double r);
    SphericalFluidSource *addSphericalFluidSource(glm::vec3 pos, double r, 
                                                  glm::vec3 velocity);
    CuboidFluidSource *addCuboidFluidSource(AABB bbox);
    CuboidFluidSource *addCuboidFluidSource(AABB bbox, glm::vec3 velocity);
    void removeFluidSource(FluidSource *source);
    void removeFluidSources();

    void addSolidCell(int i, int j, int k);
    void addSolidCells(std::vector<glm::vec3> indices);
    void removeSolidCell(int i, int j, int k);
    void removeSolidCells(std::vector<glm::vec3> indices);
    std::vector<glm::vec3> getSolidCells();
    std::vector<glm::vec3> getSolidCellPositions();

    unsigned int getNumMarkerParticles();
    std::vector<glm::vec3> getMarkerParticlePositions();
    std::vector<glm::vec3> getMarkerParticleVelocities();
    std::vector<DiffuseParticle> getDiffuseParticles();
    Array3d<float> getDensityGrid();
    MACVelocityField* getVelocityField();
    LevelSet* getLevelSet();
    TriangleMesh* getFluidSurfaceTriangles();

private:

    struct DiffuseParticleEmitter {
        glm::vec3 position;
        glm::vec3 velocity;
        double energyPotential;
        double wavecrestPotential;
        double turbulencePotential;

        DiffuseParticleEmitter() : position(0.0, 0.0, 0.0),
                                   velocity(0.0, 0.0, 0.0),
                                   energyPotential(0.0),
                                   wavecrestPotential(0.0),
                                   turbulencePotential(0.0) {}

        DiffuseParticleEmitter(glm::vec3 p, glm::vec3 v, 
                               double e, double wc, double t) : 
                                   position(p),
                                   velocity(v),
                                   energyPotential(e),
                                   wavecrestPotential(wc),
                                   turbulencePotential(t) {}
    };    

    struct CellFace {
        glm::vec3 normal;
        double minx, maxx;
        double miny, maxy;
        double minz, maxz;

        CellFace() : normal(0.0, 0.0, 0.0),
                     minx(0.0), maxx(0.0),
                     miny(0.0), maxy(0.0),
                     minz(0.0), maxz(0.0) {}

        CellFace(glm::vec3 n, glm::vec3 minp, glm::vec3 maxp) :
                     normal(n), 
                     minx(minp.x), maxx(maxp.x),
                     miny(minp.y), maxy(maxp.y),
                     minz(minp.z), maxz(maxp.z) {}
    };

    struct FluidPoint {
        glm::vec3 position;
        double radius;

        FluidPoint() : position(0.0, 0.0, 0.0),
                       radius(0.0) {}
        FluidPoint(glm::vec3 p, double r) : position(p),
                                            radius(r) {}
    };

    struct FluidCuboid {
        AABB bbox;

        FluidCuboid() : bbox(glm::vec3(0.0, 0.0, 0.0), 0.0, 0.0, 0.0) {}
        FluidCuboid(glm::vec3 p, double w, double h, double d) : 
                        bbox(p, w, h, d) {}
    };

    struct MatrixCoefficients {
        Array3d<float> diag;
        Array3d<float> plusi;
        Array3d<float> plusj;
        Array3d<float> plusk;
        int width, height, depth;

        MatrixCoefficients() : width(0), height(0), depth(0) {} 
        MatrixCoefficients(int i, int j, int k) : 
                                diag(Array3d<float>(i, j, k, 0.0f)),
                                plusi(Array3d<float>(i, j, k, 0.0f)),
                                plusj(Array3d<float>(i, j, k, 0.0f)),
                                plusk(Array3d<float>(i, j, k, 0.0f)),
                                width(i), height(j), depth(k) {};
    };

    struct VectorCoefficients {
        Array3d<float> vector;
        int width, height, depth;

        VectorCoefficients() : width(0), height(0), depth(0) {}
        VectorCoefficients(int i, int j, int k) : 
                                vector(Array3d<float>(i, j, k, 0.0f)),
                                width(i), height(j), depth(k) {}
    };

    // Type constants
    int M_AIR = 0;
    int M_FLUID = 1;
    int M_SOLID = 2;
    int T_INFLOW = 0;
    int T_OUTFLOW = 1;
    int DP_BUBBLE = 0;
    int DP_FOAM = 1;
    int DP_SPRAY = 2;

    // Initialization before running simulation
    void _initializeSimulation();
    void _initializeSolidCells();
    void _initializeFluidMaterial();
    void _getInitialFluidCellsFromImplicitSurface(std::vector<GridIndex> &fluidCells);
    void _getInitialFluidCellsFromTriangleMesh(std::vector<GridIndex> &fluidCells);
    void _addMarkerParticlesToCell(GridIndex g);
    void _addMarkerParticlesToCell(GridIndex g, glm::vec3 velocity);
    void _initializeSimulationFromSaveState(FluidSimulationSaveState &state);
    void _initializeMarkerParticlesFromSaveState(FluidSimulationSaveState &state);
    void _initializeFluidMaterialParticlesFromSaveState();
    void _initializeSolidCellsFromSaveState(FluidSimulationSaveState &state);

    // Simulation step
    double _calculateNextTimeStep();
    double _getMaximumMarkerParticleSpeed();
    void _stepFluid(double dt);

    // Find fluid cells. Fluid cells must contain at
    // least 1 marker particle
    int _getUniqueFluidSourceID();
    void _updateFluidCells();
    void _updateFluidSources();
    void _updateFluidSource(FluidSource *source);
    void _addNewFluidCells(std::vector<GridIndex> &cells, glm::vec3 velocity);
    void _removeMarkerParticlesFromCells(std::vector<GridIndex> &cells);
    inline bool _isIndexInList(GridIndex g, std::vector<GridIndex> &list) {
        GridIndex c;
        for (unsigned int idx = 0; idx < list.size(); idx++) {
            c = list[idx];
            if (g.i == c.i && g.j == c.j && g.k == c.k) {
                return true;
            }
        }
        return false;
    }

    // Convert marker particles to fluid surface
    void _reconstructFluidSurface();
    TriangleMesh _polygonizeSurface();

    // Update level set surface
    void _updateLevelSetSignedDistance();

    // Reconstruct output fluid surface
    void _reconstructOutputFluidSurface(double dt);
    void _writeSurfaceMeshToFile(TriangleMesh &mesh);
    void _writeDiffuseMaterialToFile(std::string bubblefile,
                                     std::string foamfile,
                                     std::string sprayfile);
    void _writeSmoothTriangleListToFile(TriangleMesh &mesh, std::string filename);
    void _writeBrickColorListToFile(TriangleMesh &mesh, std::string filename);
    void _writeBrickMaterialToFile(std::string brickfile, std::string colorfile);
    void _smoothSurfaceMesh(TriangleMesh &mesh);
    void _getSmoothVertices(TriangleMesh &mesh, std::vector<int> &smoothVertices);
    bool _isVertexNearSolid(glm::vec3 v, double eps);
    void _updateSmoothTriangleList(TriangleMesh &mesh, std::vector<int> &smoothVertices);
    TriangleMesh _polygonizeOutputSurface();
    void _getSubdividedSurfaceCells(std::vector<GridIndex> &cells);
    void _getSubdividedSolidCells(std::vector<GridIndex> &cells);
    void _getOutputSurfaceParticles(std::vector<glm::vec3> &particles);
    void _updateBrickGrid(double dt);

    // Advect fluid velocities
    void _advectVelocityField();
    void _advectVelocityFieldU();
    void _advectVelocityFieldV();
    void _advectVelocityFieldW();
    void _computeVelocityScalarField(Array3d<float> &field,
                                     Array3d<float> &weightfield, 
                                     int dir);

    // Add gravity to fluid velocities
    void _applyBodyForcesToVelocityField(double dt);

    // Extrapolate fluid velocities into surrounding air and solids so
    // that velocities can be computed when marker particles move to cells
    // outside of current fluid region
    void _extrapolateFluidVelocities();
    void _resetExtrapolatedFluidVelocities();
    int _updateExtrapolationLayers(Array3d<int> &layerGrid);
    void _updateExtrapolationLayer(int layerIndex, Array3d<int> &layerGrid);
    void _extrapolateVelocitiesForLayerIndex(int layerIndex, Array3d<int> &layerGrid);
    void _extrapolateVelocitiesForLayerIndexU(int layerIndex, Array3d<int> &layerGrid);
    void _extrapolateVelocitiesForLayerIndexV(int layerIndex, Array3d<int> &layerGrid);
    void _extrapolateVelocitiesForLayerIndexW(int layerIndex, Array3d<int> &layerGrid);
    double _getExtrapolatedVelocityForFaceU(int i, int j, int k, int layerIndex,
                                            Array3d<int> &layerGrid);
    double _getExtrapolatedVelocityForFaceV(int i, int j, int k, int layerIndex,
                                            Array3d<int> &layerGrid);
    double _getExtrapolatedVelocityForFaceW(int i, int j, int k, int layerIndex,
                                            Array3d<int> &layerGrid);
    glm::vec3 _getVelocityAtNearestPointOnFluidSurface(glm::vec3 p);
    glm::vec3 _getVelocityAtPosition(glm::vec3 p);

    // Calculate pressure values to satisfy incompressibility condition
    void _updatePressureGrid(Array3d<float> &pressureGrid, double dt);
    double _calculateNegativeDivergenceVector(VectorCoefficients &b);
    void _calculateMatrixCoefficients(MatrixCoefficients &A, double dt);
    void _calculatePreconditionerVector(VectorCoefficients &precon, MatrixCoefficients &A);
    Eigen::VectorXd _applyPreconditioner(Eigen::VectorXd r, 
                                         VectorCoefficients &precon,
                                         MatrixCoefficients &A);
    Eigen::VectorXd _solvePressureSystem(MatrixCoefficients &A, 
                                         VectorCoefficients &b, 
                                         VectorCoefficients &precon,
                                         Array3d<int> &vectorIndexHashTable,
                                         double dt);

    // Methods for setting up system of equations for the pressure update
    void _EigenVectorXdToVectorCoefficients(Eigen::VectorXd v, VectorCoefficients &vc);
    Eigen::VectorXd _VectorCoefficientsToEigenVectorXd(VectorCoefficients &p,
                                                       std::vector<GridIndex> indices);
    Eigen::SparseMatrix<double> _MatrixCoefficientsToEigenSparseMatrix(MatrixCoefficients &A,
                                                                       Array3d<int> &vectorIndexHashTable,
                                                                       double dt);
    void _updateFluidGridIndexToEigenVectorXdIndexHashTable(Array3d<int> &hashTable);
    int _GridIndexToVectorIndex(int i, int j, int k, Array3d<int> &hashTable);
    int _GridIndexToVectorIndex(GridIndex index, Array3d<int> &hashTable);
    GridIndex _VectorIndexToGridIndex(int index);
    int _getNumFluidOrAirCellNeighbours(int i, int j, int k);

    // Alter fluid velocities according to calculated pressures
    // to create a divercence free velocity field
    void _applyPressureToVelocityField(Array3d<float> &pressureGrid, double dt);
    void _applyPressureToFaceU(int i, int j, int k, Array3d<float> &pressureGrid,
                                                    MACVelocityField &tempMACVelocity, double dt);
    void _applyPressureToFaceV(int i, int j, int k, Array3d<float> &pressureGrid,
                                                    MACVelocityField &tempMACVelocity, double dt);
    void _applyPressureToFaceW(int i, int j, int k, Array3d<float> &pressureGrid,
                                                    MACVelocityField &tempMACVelocity, double dt);
    void _commitTemporaryVelocityFieldValues(MACVelocityField &tempMACVelocity);

    // Update diffuse material (spray, foam, bubbles)
    void _updateDiffuseMaterial(double dt);
    void _sortMarkerParticlePositions(std::vector<glm::vec3> &surface, 
                                      std::vector<glm::vec3> &inside);
    void _getDiffuseParticleEmitters(std::vector<DiffuseParticleEmitter> &emitters);
    void _getSurfaceDiffuseParticleEmitters(std::vector<glm::vec3> &surface, 
                                            std::vector<DiffuseParticleEmitter> &emitters);
    void _getInsideDiffuseParticleEmitters(std::vector<glm::vec3> &inside, 
                                           std::vector<DiffuseParticleEmitter> &emitters);
    double _getWavecrestPotential(glm::vec3 p, glm::vec3 *velocity);
    double _getTurbulencePotential(glm::vec3 p, TurbulenceField &tfield);
    double _getEnergyPotential(glm::vec3 velocity);
    void _emitDiffuseParticles(std::vector<DiffuseParticleEmitter> &emitters, double dt);
    void _emitDiffuseParticles(DiffuseParticleEmitter &emitter, double dt);
    int _getNumberOfEmissionParticles(DiffuseParticleEmitter &emitter,
                                       double dt);
    void _updateDiffuseParticleTypesAndVelocities();
    int _getDiffuseParticleType(DiffuseParticle &p);
    void _updateDiffuseParticleLifetimes(double dt);
    void _advanceDiffuseParticles(double dt);
    void _getNextBubbleDiffuseParticle(DiffuseParticle &dp,
                                       DiffuseParticle &nextdp,double dt);
    void _getNextSprayDiffuseParticle(DiffuseParticle &dp,
                                      DiffuseParticle &nextdp,double dt);
    void _getNextFoamDiffuseParticle(DiffuseParticle &dp,
                                     DiffuseParticle &nextdp,double dt);

    // Transfer grid velocity to marker particles
    void _updateMarkerParticleVelocities(MACVelocityField &savedField);

    // Move marker particles through the velocity field
    void _advanceMarkerParticles(double dt);
    void _advanceRangeOfMarkerParticles(int startIdx, int endIdx, double dt);
    void _removeMarkerParticles();
    void _shuffleMarkerParticleOrder();
    void _sortMarkerParticlesByGridIndex();

    // Methods for finding collisions between marker particles and solid cell
    // boundaries. Also used for advecting fluid when particle enters a solid.
    std::vector<CellFace> _getNeighbourSolidCellFaces(int i, int j, int k);
    bool _isPointOnCellFace(glm::vec3 p, CellFace f, double eps);
    bool _isPointOnSolidBoundary(glm::vec3 p, CellFace *f, double eps);
    CellFace _getCellFace(int i, int j, int k, glm::vec3 normal);
    void _getCellFaces(int i, int j, int k, CellFace[6]);
    bool _getVectorFaceIntersection(glm::vec3 p0, glm::vec3 normal, CellFace f, glm::vec3 *intersect);
    glm::vec3 _calculateSolidCellCollision(glm::vec3 p0, glm::vec3 p1, glm::vec3 *normal);
    std::vector<CellFace> _getSolidCellFaceCollisionCandidates(int i, int j, int k, glm::vec3 dir);
    bool _findFaceCollision(glm::vec3 p0, glm::vec3 p1, CellFace *face, glm::vec3 *intersection);
    
    // Runge-Kutta integrators used in advection and advancing marker particles
    glm::vec3 _RK2(glm::vec3 p0, glm::vec3 v0, double dt);
    glm::vec3 _RK3(glm::vec3 p0, glm::vec3 v0, double dt);
    glm::vec3 _RK4(glm::vec3 p0, glm::vec3 v0, double dt);

    // misc bool functions for checking cell contents and borders
    inline bool _isCellAir(int i, int j, int k) { return _materialGrid(i, j, k) == M_AIR; }
    inline bool _isCellFluid(int i, int j, int k) { return _materialGrid(i, j, k) == M_FLUID; }
    inline bool _isCellSolid(int i, int j, int k) { return _materialGrid(i, j, k) == M_SOLID; }
    inline bool _isCellAir(GridIndex g) { return _materialGrid(g) == M_AIR; }
    inline bool _isCellFluid(GridIndex g) { return _materialGrid(g) == M_FLUID; }
    inline bool _isCellSolid(GridIndex g) { return _materialGrid(g) == M_SOLID; }
    

    inline bool _isFaceBorderingGridValueU(int i, int j, int k, int value, Array3d<int> &grid) {
        if (i == grid.width) { return grid(i - 1, j, k) == value; }
        else if (i > 0) { return grid(i, j, k) == value || grid(i - 1, j, k) == value; }
        else { return grid(i, j, k) == value; }
    }
    inline bool _isFaceBorderingGridValueV(int i, int j, int k, int value, Array3d<int> &grid) {
        if (j == grid.height) { return grid(i, j - 1, k) == value; }
        else if (j > 0) { return grid(i, j, k) == value || grid(i, j - 1, k) == value; }
        else { return grid(i, j, k) == value; }
    }
    inline bool _isFaceBorderingGridValueW(int i, int j, int k, int value, Array3d<int> &grid) {
        if (k == grid.depth) { return grid(i, j, k - 1) == value; }
        else if (k > 0) { return grid(i, j, k) == value || grid(i, j, k - 1) == value; }
        else { return grid(i, j, k) == value; }
    }

    inline bool _isFaceBorderingGridValueU(GridIndex g, int value, Array3d<int> &grid) {
        return _isFaceBorderingGridValueU(g.i, g.j, g.k, value, grid);
    }
    inline bool _isFaceBorderingGridValueV(GridIndex g, int value, Array3d<int> &grid) {
        return _isFaceBorderingGridValueV(g.i, g.j, g.k, value, grid);
    }
    inline bool _isFaceBorderingGridValueW(GridIndex g, int value, Array3d<int> &grid) {
        return _isFaceBorderingGridValueW(g.i, g.j, g.k, value, grid);
    }

    inline bool _isFaceBorderingMaterialU(int i, int j, int k, int mat) {
        return _isFaceBorderingGridValueU(i, j, k, mat, _materialGrid);
    }
    inline bool _isFaceBorderingMaterialV(int i, int j, int k, int mat) {
        return _isFaceBorderingGridValueV(i, j, k, mat, _materialGrid);
    }
    inline bool _isFaceBorderingMaterialW(int i, int j, int k, int mat) {
        return _isFaceBorderingGridValueW(i, j, k, mat, _materialGrid);
    }

    inline bool _isFaceBorderingLayerIndexU(int i, int j, int k, int layer, Array3d<int> &layerGrid) {
        return _isFaceBorderingGridValueU(i, j, k, layer, layerGrid);
    }
    inline bool _isFaceBorderingLayerIndexV(int i, int j, int k, int layer, Array3d<int> &layerGrid) {
        return _isFaceBorderingGridValueV(i, j, k, layer, layerGrid);
    }
    inline bool _isFaceBorderingLayerIndexW(int i, int j, int k, int layer, Array3d<int> &layerGrid) {
        return _isFaceBorderingGridValueW(i, j, k, layer, layerGrid);
    }
    inline bool _isFaceBorderingLayerIndexU(GridIndex g, int layer, Array3d<int> &layerGrid) {
        return _isFaceBorderingGridValueU(g, layer, layerGrid);
    }
    inline bool _isFaceBorderingLayerIndexV(GridIndex g, int layer, Array3d<int> &layerGrid) {
        return _isFaceBorderingGridValueV(g, layer, layerGrid);
    }
    inline bool _isFaceBorderingLayerIndexW(GridIndex g, int layer, Array3d<int> &layerGrid) {
        return _isFaceBorderingGridValueW(g, layer, layerGrid);
    }

    inline bool _isFaceVelocityExtrapolatedU(int i, int j, int k, Array3d<int> &layerGrid) {
        if (i == _isize) {  return layerGrid(i - 1, j, k) >= 1.0; }
        else if (i > 0) { return layerGrid(i, j, k) >= 1.0 || layerGrid(i - 1, j, k) >= 1.0; }
        else { return layerGrid(i, j, k) >= 1.0; }
    }

    inline bool _isFaceVelocityExtrapolatedV(int i, int j, int k, Array3d<int> &layerGrid) {
        if (j == _jsize) { return layerGrid(i, j - 1, k) >= 1.0; }
        else if (j > 0) { return layerGrid(i, j, k) >= 1.0 || layerGrid(i, j - 1, k) >= 1.0; }
        else { return layerGrid(i, j, k) >= 1.0; }
    }

    inline bool _isFaceVelocityExtrapolatedW(int i, int j, int k, Array3d<int> &layerGrid) {
        if (k == _ksize) { return layerGrid(i, j, k - 1) >= 1.0; }
        else if (k > 0) { return layerGrid(i, j, k) >= 1.0 || layerGrid(i, j, k - 1) >= 1.0; }
        else { return layerGrid(i, j, k) >= 1.0; }
    }

    inline double _randomFloat(double min, double max) {
        return min + static_cast <double> (rand()) / (static_cast <double> (RAND_MAX / (max - min)));
    }

    bool _isSimulationInitialized = false;
    bool _isSimulationRunning = false;
    bool _isFluidInSimulation = false;
    int _currentFrame = 0;
    int _currentTimeStep = 0;
    double _frameTimeStep = 0.0;
    bool _isCurrentFrameFinished = true;
    bool _isLastTimeStepForFrame = false;
    double _simulationTime = 0;
    double _realTime = 0;

    double _dx = 0.0;
    double _density = 20.0;
    int _isize = 0;
    int _jsize = 0;
    int _ksize = 0;

    double _CFLConditionNumber = 5.0;
    double _minTimeStep = 1.0 / 300.0;
    double _maxTimeStep = 1.0 / 15.0;
    double _maxAdvectionDistanceFactor = 2.5; // max number of cells an advection
                                              // integration can travel
    double _pressureSolveTolerance = 10e-6;
    int _maxPressureSolveIterations = 150;
    int _numAdvanceMarkerParticleThreads = 8;

    double _surfaceReconstructionSmoothingValue = 0.85;
    int _surfaceReconstructionSmoothingIterations = 3;
    double _markerParticleRadius;
    double _markerParticleScale = 3.0;

    int _outputFluidSurfaceSubdivisionLevel = 1;
    double _outputFluidSurfaceCellNarrowBandSize = 0.5;
    double _outputFluidSurfaceParticleNarrowBandSize = 1.0;

    double _diffuseSurfaceNarrowBandSize = 0.25; // size in # of cells
    double _minWavecrestCurvature = 0.20;
    double _maxWavecrestCurvature = 1.0;
    double _minParticleEnergy = 0.0;
    double _maxParticleEnergy = 20.0;
    double _minTurbulence = 100.0;
    double _maxTurbulence = 200.0;
    double _wavecrestEmissionRate = 200.0;
    double _turbulenceEmissionRate = 200.0;
    double _maxDiffuseParticleLifetime = 2.0;
    double _maxFoamToSurfaceDistance = 1.0;   // in number of grid cells
    double _minBubbleToSurfaceDistance = 1.0; // in number of grid cells
    double _bubbleBouyancyCoefficient = 4.0;
    double _bubbleDragCoefficient = 1.0;
    double _maxFlatCurvature = 0.05;

    double _minBrickNeighbourRatio = 0.10;
    double _maxBrickNeighbourRatio = 0.50;
    double _brickNeighbourIntensityInfluenceRatio = 0.5;
    double _maxBrickIntensityVelocity = 10.0;
    double _maxBrickIntensityAcceleration = 10.0;
    int _maxInactiveBrickFrames = 0;

    double _ratioPICFLIP = 0.35f;
    int _maxMarkerParticlesPerCell = 50;

    bool _isSurfaceMeshOutputEnabled = true;
    bool _isDiffuseMaterialOutputEnabled = false;
    bool _isBrickOutputEnabled = false;
    double _brickWidth = 1.0;
    double _brickHeight = 1.0;
    double _brickDepth = 1.0;
    int _currentBrickMeshFrame = 0;

    glm::vec3 _bodyForce;

    int MESH = 0;
    int IMPLICIT = 1;
    int _fluidInitializationType = 2;
    std::string _fluidMeshFilename;
    glm::vec3 _fluidMeshOffset;
    double _fluidMeshScale = 1.0;

    MACVelocityField _MACVelocity;
    Array3d<int> _materialGrid;
    std::vector<MarkerParticle> _markerParticles;
    std::vector<GridIndex> _fluidCellIndices;
    LogFile _logfile;
    TriangleMesh _surfaceMesh;
    LevelSet _levelset;
    std::vector<bool> _isSurfaceTriangleSmooth;
    
    std::vector<FluidPoint> _fluidPoints;
    std::vector<FluidCuboid> _fluidCuboids;
    std::vector<FluidSource*> _fluidSources;
    std::vector<SphericalFluidSource*> _sphericalFluidSources;
    std::vector<CuboidFluidSource*> _cuboidFluidSources;
    int _uniqueFluidSourceID = 0;
    TurbulenceField _turbulenceField;
    std::vector<DiffuseParticle> _diffuseParticles;

    Array3d<Brick> _brickGrid;

    FluidBrickGrid _fluidBrickGrid;
};