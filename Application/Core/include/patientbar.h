/*-----------------------------------------------------------------------------
 | Copyright: Copyright(c) 2015, Philips Medical Systems Nederland B.V.       |
 | Author:    I.W.R.A. Limpens                                                |
 -----------------------------------------------------------------------------*/

#pragma once

#include <Sense.h>
#include <ExperienceIdentity.h>

namespace Pci { namespace Core
{

using namespace::Sense;
using namespace::Sense::ExperienceIdentity;

enum class PatientBarColorScheme
{
	Bright,
	Dark,
	SmS
};

struct PatientInfo
{
	std::wstring name;
	std::wstring id;
	std::wstring birthdate;

	bool operator==(const PatientInfo& other) const
	{
		return name == other.name && id == other.id && birthdate == other.birthdate;
	}

	bool operator!= (const PatientInfo& other) const
	{
		return !(*this == other);
	}
};

class PatientBarHeader : public Control
{
public:
	explicit PatientBarHeader(Control &parent, PatientBarColorScheme scheme = PatientBarColorScheme::Bright);
	virtual ~PatientBarHeader() {}

	Property<PatientInfo> patientData;

	PatientBarHeader(const PatientBarHeader&) = delete;
	PatientBarHeader& operator=(const PatientBarHeader&) = delete;
protected:
	virtual void render(IRenderer &renderer) const override;
	virtual void onResize() override;

private:
	void arrangePatientInfo() const;
	void onPatientDataChanged();
	void applyColorScheme(PatientBarColorScheme scheme);

private:
	mutable Label		patientName;
	mutable Text		patientIdLabel;
	mutable Label		patientId;
	mutable Text		patientDobLabel;
	mutable Label		patientDob;
};

class PatientBar : public PatientBarHeader
{
public:
	explicit PatientBar(Control &parent, PatientBarColorScheme scheme = PatientBarColorScheme::Bright);
	virtual ~PatientBar() {}

	Property<Corners>		radius;				  //!< Amount of rounding at the corners.
	Property<Margin>		border;				  //!< Size of the border.
	Property<Margin>		edge;				  //!< Size of the edge.
	Property<Margin>		shadow;				  //!< Size of the shadow.

	PatientBar(const PatientBar&) = delete;
	PatientBar& operator=(const PatientBar&)= delete;
protected:
	virtual void render(IRenderer &renderer) const override;

private:
	mutable Background  background;
	mutable Background  dimmedBackground;
};

}}
