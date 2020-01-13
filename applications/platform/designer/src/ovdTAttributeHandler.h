#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class TAttributeHandler
	{
	public:

		explicit TAttributeHandler(OpenViBE::Kernel::IAttributable& attributable) : m_constAttributable(&attributable), m_attributable(&attributable) { }
		explicit TAttributeHandler(const OpenViBE::Kernel::IAttributable& attributable) : m_constAttributable(&attributable) { }

		template <class T>
		bool addAttribute(const OpenViBE::CIdentifier& id, const T& value) const;

		bool removeAttribute(const OpenViBE::CIdentifier& id);

		bool removeAllAttributes();

		template <class T>
		T getAttributeValue(const OpenViBE::CIdentifier& id) const;

		template <class T>
		bool setAttributeValue(const OpenViBE::CIdentifier& id, const T& value);

		bool hasAttribute(const OpenViBE::CIdentifier& id) const { return m_constAttributable->hasAttribute(id); }
		bool hasAttributes() const { return m_constAttributable->hasAttributes(); }

	protected:

		const OpenViBE::Kernel::IAttributable* m_constAttributable;
		OpenViBE::Kernel::IAttributable* m_attributable = nullptr;
	};
}  // namespace OpenViBEDesigner
