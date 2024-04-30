#include "solid_body.h"
#include "base_material.h"
#include "solid_particles.h"
#include "sph_system.h"

namespace SPH
{
//=================================================================================================//
SolidBodyPartForSimbody::
    SolidBodyPartForSimbody(SPHBody &body, Shape &body_part_shape)
    : BodyRegionByParticle(body, body_part_shape),
      rho0_(DynamicCast<Solid>(this, body.base_material_)->ReferenceDensity()),
      Vol_(*base_particles_.getVariableByName<Real>("VolumetricMeasure")),
      pos_(*base_particles_.getVariableByName<Vecd>("Position"))
{
    setMassProperties();
}
//=================================================================================================//
SolidBodyPartForSimbody::SolidBodyPartForSimbody(SPHBody &body, SharedPtr<Shape> shape_ptr)
    : SolidBodyPartForSimbody(body, *shape_ptr.get()) {}
//=================================================================================================//
} // namespace SPH
