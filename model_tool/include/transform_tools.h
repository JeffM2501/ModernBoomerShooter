#pragma once
#include "raylib.h"

#include "app.h"

namespace TransformTools
{
    void CenterMesh();
    void FloorMesh();
    void RotateMesh(float angle, Vector3 axis);
    void BakeMaterialColors();
    void CombineIdenticalMaterials();
    void MergeMeshes();
}

class TransformPanel : public Panel
{
public:
    TransformPanel();

protected:
    void OnShowContents() override;
};