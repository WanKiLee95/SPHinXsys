/**
 * @file 	complex_body_relation.cpp
 * @author	Chi ZHang and Xiangyu Hu
 */

#include "complex_body_relation.h"

#include "inner_body_relation.h"
#include "contact_body_relation.h"
#include "base_particle_dynamics.h"

namespace SPH
{
	//=================================================================================================//
	ComplexRelation::
		ComplexRelation(BaseInnerRelation &inner_relation, BaseContactRelation &contact_relation)
		: SPHRelation(inner_relation.sph_body_),
		  inner_relation_(inner_relation),
		  contact_relation_(contact_relation),
		  contact_bodies_(contact_relation_.contact_bodies_),
		  inner_configuration_(inner_relation_.inner_configuration_),
		  contact_configuration_(contact_relation_.contact_configuration_)
	{
		updateConfigurationMemories();
	}
	//=================================================================================================//
	ComplexRelation::ComplexRelation(RealBody &real_body, RealBodyVector contact_bodies)
		: SPHRelation(real_body),
		  inner_relation_(base_inner_relation_ptr_keeper_.createRef<InnerRelation>(real_body)),
		  contact_relation_(base_contact_relation_ptr_keeper_
								.createRef<ContactRelation>(real_body, contact_bodies)),
		  contact_bodies_(contact_relation_.contact_bodies_),
		  inner_configuration_(inner_relation_.inner_configuration_),
		  contact_configuration_(contact_relation_.contact_configuration_)
	{
		updateConfigurationMemories();
	}
	//=================================================================================================//
	ComplexRelation::
		ComplexRelation(BaseInnerRelation &inner_relation, RealBodyVector contact_bodies)
		: SPHRelation(inner_relation.sph_body_),
		  inner_relation_(inner_relation),
		  contact_relation_(base_contact_relation_ptr_keeper_.createRef<ContactRelation>(
			  DynamicCast<RealBody>(this, sph_body_), contact_bodies)),
		  contact_bodies_(contact_relation_.contact_bodies_),
		  inner_configuration_(inner_relation_.inner_configuration_),
		  contact_configuration_(contact_relation_.contact_configuration_)
	{
		updateConfigurationMemories();
	}
	//=================================================================================================//
	ComplexRelation::ComplexRelation(RealBody &real_body, BodyPartVector contact_body_parts)
		: SPHRelation(real_body),
		  inner_relation_(base_inner_relation_ptr_keeper_.createRef<InnerRelation>(real_body)),
		  contact_relation_(base_contact_relation_ptr_keeper_
								.createRef<ContactRelationToBodyPart>(real_body, contact_body_parts)),
		  contact_bodies_(contact_relation_.contact_bodies_),
		  inner_configuration_(inner_relation_.inner_configuration_),
		  contact_configuration_(contact_relation_.contact_configuration_)
	{
		updateConfigurationMemories();
	}
	//=================================================================================================//
	void ComplexRelation::updateConfigurationMemories()
	{
		inner_relation_.updateConfigurationMemories();
		contact_relation_.updateConfigurationMemories();
	}
	//=================================================================================================//
	void ComplexRelation::updateConfiguration()
	{
		inner_relation_.updateConfiguration();
		contact_relation_.updateConfiguration();
	}
	//=================================================================================================//
}
