// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "ICrmAdministration.h"

#include "XrayGeometry.h"
#include "CrmRoadmap.h"
#include "CrmParameters.h"
#include "CrmErrors.h"

namespace CommonPlatform
{
    class Log;
}

namespace Pci { namespace Core {

static const int      MaxNumberOfRoadmaps = 1024;

class CrmRepository;

struct Persistent
{
	static const int Version = 3;

	int		version;
	int		runIndex; // made persistent to continue counting both fluoro and cine runs after restart.
	wchar_t studyID[512];
	int		roadmapCount;
	int		roadmaps[MaxNumberOfRoadmaps];
	bool    automatic3dApcActivation;
};

// This class implements the administration and storage of data for coronary road mapping
class CrmAdministration : public ICrmAdministration
{
public:
	CrmAdministration(const CrmParameters &params, CrmRepository &repository);
	CrmAdministration( CommonPlatform::Log& log, const CrmParameters &params, CrmRepository &repository);
	virtual ~CrmAdministration();
	
    // Status of the roadmap administration
	enum class Status 
    { 
        // NoRoadmap: There is no roadmap available that can be used in the current geometry position.
        NoRoadmap, 
        /// MoveArc:   There is a roadmap available, but it can not be activated until the C-arc is moved towards the correct position.
        MoveArc, 
        // MoveTable: There is a roadmap available, but it can not be activated until the patient support is moved towards the correct position.
        MoveTable, 
        // Active:    There is a roadmap available that can be used; the geometry is in a correct position.
        Active
    };

    // Add a roadmap (a processed angio run) to the administration.
	void addRoadmap(const std::shared_ptr<const CrmRoadmap> &roadmap);

    // Returns the roadmap selected after geometry and study changes.
	const std::shared_ptr<const CrmRoadmap>& getRoadmap() const;

    // Returns the current status of the administration.
    Status getStatus() const;

    // Returns true when the administration does not contains roadmaps.
	bool isEmpty() const;

    // Pass new information to the administration when the geometry changes.
	void setGeometry(const CommonPlatform::Xray::XrayGeometry &geometry);

    // Notify the administration that the geometry has stopped moving. (E.g. after several calls to setGeometry() are made)
    void endGeometry();

    // Pass new information to the administration when the patient data changes.
	void setStudy   (const CommonPlatform::Xray::XrayStudy    &study);
	
    // Notify the administration that an incoming cine or fluoro run has ended.
	void endLive();

    // Return the index of the last handled run. This value is kept persistent by the CrmAdministration and can be used
    // to override run index for from XraySourceRto and could be used for all PCI express applications.
    // TODO remove when grabber is removed from product code. 
    // To identify individual runs we should use the run number from the Allura (which is in the DVLP header).
	int getPersistentRunIndex() const;

    // Return ID of the last known study.
    std::wstring getStudyID() const;

    // Event raised by administration when the selected roadmap or status has changed.
	std::function<void()> onRoadmapStatusChanged;

    // Return the version ID that identified the format of the persistent data.
    int getPersistentDataVersion() const;

    // Check for roadmap with better quality and overwrite if needed.
    PciSuite::CrmErrors::Error testRoadmap(const std::shared_ptr<const CrmRoadmap> &roadmap);

    // Access to the persistent data of the App is handled by this Class.
    void setPersistent3dApcAutomaticActivation(bool enabled);
    bool getPersistent3dApcAutomaticActivation() const;

	// Add image and overlay to the recording.
	void setImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image, const std::shared_ptr<const CrmOverlay> &roadmap);

	// Return the current/last recording
	std::shared_ptr<CrmRecording> getRecording() const override;

	std::vector<std::shared_ptr<const CrmRoadmap>> getRoadmapsForCurrentGeo() const override;

	void selectRoadmap(std::shared_ptr<const CrmRoadmap>& roadmap) override;

	void lockRoadmap(std::shared_ptr<const CrmRoadmap>& roadmap) override;
	void unlockRoadmap(std::shared_ptr<const CrmRoadmap>& roadmap) override;

	CrmAdministration(const CrmAdministration&) = delete;
	CrmAdministration& operator=(const CrmAdministration&) = delete;

protected:
    Persistent*                 m_persistentView;
    std::vector<std::shared_ptr<const CrmRoadmap>> m_roadmaps;

private:
    virtual void updatePersistent       (); // virtual only for test purposes
	bool update                 ();
    void createAndLoadPersistent();
    bool loadPersistent         ();
    std::shared_ptr<const CrmRoadmap> findBestMatchingRoadmap(const CommonPlatform::Xray::XrayGeometry &geometryPosition) const;

	std::vector<std::shared_ptr<const CrmRoadmap>> findMatchingRoadmaps(const XrayGeometry &geometry) const;

	HANDLE								m_persistentMapping;

	CommonPlatform::Log&				m_log;
	CrmRepository&						m_repository;
	CommonPlatform::Xray::XrayGeometry	m_geometryPosition;
	bool						        m_geometryMoving;
	Status					        	m_status;
	std::shared_ptr<const CrmRoadmap>	m_currentRoadmap;
	const CrmParameters					&m_params;
	int									m_persistentRunIndex;
    std::wstring						m_studyID;
    bool							    m_automatic3dApcActivation;

	std::shared_ptr<CrmRecording>       m_recording;
};

}}
