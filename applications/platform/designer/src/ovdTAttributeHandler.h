#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class TAttributeHandler
	{
	public:

		TAttributeHandler(OpenViBE::Kernel::IAttributable& attributable) : m_pConstAttributable(&attributable), m_pAttributable(&attributable) { }
		TAttributeHandler(const OpenViBE::Kernel::IAttributable& attributable) : m_pConstAttributable(&attributable) { }

		template <class T>
		bool addAttribute(const OpenViBE::CIdentifier& rAttributeIdentifier, const T& rValue) const;

		bool removeAttribute(const OpenViBE::CIdentifier& rAttributeIdentifier);

		bool removeAllAttributes();

		template <class T>
		T getAttributeValue(const OpenViBE::CIdentifier& rAttributeIdentifier) const;

		template <class T>
		bool setAttributeValue(const OpenViBE::CIdentifier& rAttributeIdentifier, const T& rValue);

		bool hasAttribute(const OpenViBE::CIdentifier& identifier) const { return m_pConstAttributable->hasAttribute(identifier); }
		bool hasAttributes() const { return m_pConstAttributable->hasAttributes(); }

	protected:

		const OpenViBE::Kernel::IAttributable* m_pConstAttributable;
		OpenViBE::Kernel::IAttributable* m_pAttributable = nullptr;
	};
};
