// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>
#include <vector>

namespace CommonPlatform
{

class Settings
{
public:
	explicit Settings(const std::wstring &file);
	virtual ~Settings() {}
	
	std::wstring get(const wchar_t *group, const wchar_t *name, const wchar_t  *def) const;
	int          get(const wchar_t *group, const wchar_t *name, int				def) const;
	bool         get(const wchar_t *group, const wchar_t *name, bool			def) const;
	double       get(const wchar_t *group, const wchar_t *name, double			def) const;
	float        get(const wchar_t *group, const wchar_t *name, float			def) const;

	void         set(const wchar_t *group, const wchar_t *name, const std::wstring &value);
    void         set(const wchar_t *group, const wchar_t *name, const wchar_t  *value);
	void         set(const wchar_t *group, const wchar_t *name, int			   value);
	void         set(const wchar_t *group, const wchar_t *name, bool		   value);
	void         set(const wchar_t *group, const wchar_t *name, double		   value);
	void         set(const wchar_t *group, const wchar_t *name, float		   value);
	
	const wchar_t *filename();

	std::vector<std::wstring> getGroups() const;
	std::vector<std::wstring> getGroupKeys(const std::wstring& group) const;

private:

	wchar_t m_fileName[260]; // MAX_PATH

};

}