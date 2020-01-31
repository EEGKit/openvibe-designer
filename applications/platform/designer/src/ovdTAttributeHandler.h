#pragma once

#include "ovd_base.h"

namespace OpenViBE
{
	namespace Designer
	{
		class TAttributeHandler
		{
		public:

			explicit TAttributeHandler(Kernel::IAttributable& attributable) : m_constAttributable(&attributable), m_attributable(&attributable) { }
			explicit TAttributeHandler(const Kernel::IAttributable& attributable) : m_constAttributable(&attributable) { }

			template <class T>
			bool addAttribute(const CIdentifier& id, const T& value) const;

			bool removeAttribute(const CIdentifier& id);

			bool removeAllAttributes();

			template <class T>
			T getAttributeValue(const CIdentifier& id) const;

			template <class T>
			bool setAttributeValue(const CIdentifier& id, const T& value);

			bool hasAttribute(const CIdentifier& id) const { return m_constAttributable->hasAttribute(id); }
			bool hasAttributes() const { return m_constAttributable->hasAttributes(); }

		protected:

			const Kernel::IAttributable* m_constAttributable;
			Kernel::IAttributable* m_attributable = nullptr;
		};
	}  // namespace Designer
}  // namespace OpenViBE
