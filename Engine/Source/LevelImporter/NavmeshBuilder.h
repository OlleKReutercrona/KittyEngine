#pragma once

namespace KE
{
    class KittyMesh;
}

class NavmeshBuilder
{
public:    
    static KE::KittyMesh Build(std::vector<Transform>& someTransforms);
};
