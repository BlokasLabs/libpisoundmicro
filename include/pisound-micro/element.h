// SPDX-License-Identifier: LGPL-2.1-only
//
// libpisoundmicro - a utility library for Pisound Micro I/O expander capabilities.
// Copyright (c) 2017-2025 Vilniaus Blokas UAB, https://blokas.io/
//
// This file is part of libpisoundmicro.
//
// libpisoundmicro is free software: you can redistribute it and/or modify it under the terms of the
// GNU Lesser General Public License as published by the Free Software Foundation, version 2.1 of the License.
//
// libpisoundmicro is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with libpisoundmicro. If not, see <https://www.gnu.org/licenses/>.

#ifndef PISOUND_MICRO_ELEMENT_H
#define PISOUND_MICRO_ELEMENT_H

#ifndef PISOUND_MICRO_H
#	error Please #include <pisound-micro.h>!
#endif

/**
 * @file element.h
 * @brief Element class header.
 */

namespace upisnd
{

/** @brief An Element class for shared functionality of all Pisound Micro Elements.
 *
 * It takes care of adding and releaseing references to the underlying C API element,
 * it follows the usual C++ class semantics.
 *
 * It also provides a safe templated as() method for casting to the appropriate class.
 *
 * You may use the Element::setup method to create a new element from a #upisnd_setup_t setup
 * option container.
 *
 * @ingroup cpp
 */
class UPISND_API Element
{
public:
	Element();

	/// Implicitly increments the refcount. If you got the element from C API, don't forget to unref it afterwards.
	explicit Element(upisnd_element_ref_t el);

	Element(const Element &rhs);
	Element &operator=(const Element &rhs);

	Element(Element &&rhs);
	Element& operator=(Element &&rhs);

	/// Unreferences the element.
	~Element();

	/** @brief Gets an Element following ::upisnd_element_get semantics.
	 *
	 * @param name The name of the element to get. Can be provided as a string constant or `const char *`.
	 */
	static Element get(ElementName name);

	/** @brief Sets up a new Element from a #upisnd_setup_t setup option container.
	 *
	 * @param name The name of the element to get. Can be provided as a string constant or `const char *`.
	 * @param setup The setup option container. See ::upisnd_setup_set_element_type and the rest of the
	 * upisnd_setup_* APIs in api.h.
	 */
	static Element setup(ElementName name, upisnd_setup_t setup);

	/// Checks if the element is valid.
	bool isValid() const;

	/// Unreferences the underlying C #upisnd_element_ref_t handle and resets the object.
	void release();

	/// Gets the name of the element.
	const char *getName() const;

	/// Gets the type of the element.
	upisnd_element_type_e getType() const;

	/// Gets the pin of the element.
	upisnd_pin_t getPin() const;

	/** @brief Opens the Element's `value` attribute as a file descriptor.
	 *
	 * @param flags The flags to pass to ::open.
	 *
	 * @see upisnd_element_open_value_fd
	 */
	ValueFd openValueFd(int flags) const;

	/// Safely casts the element to the requested class. Returns an invalid object if the cast is not possible.
	template <typename T>
	inline T as() const
	{
		if (classType<T>() == getType())
			return T(ref());
		return T();
	}

protected:
	inline upisnd_element_ref_t ref() const { return m_ref; }

	// Copy el without changing the refcount, for internal use.
	Element(upisnd_element_ref_t el, bool);

private:
	template <typename T>
	static upisnd_element_type_e classType() { return UPISND_ELEMENT_TYPE_INVALID; };

	upisnd_element_ref_t m_ref;
};

template <> upisnd_element_type_e Element::classType<Encoder>();
template <> upisnd_element_type_e Element::classType<AnalogInput>();
template <> upisnd_element_type_e Element::classType<Gpio>();
template <> upisnd_element_type_e Element::classType<Activity>();

} // namespace upisnd

#endif // PISOUND_MICRO_ELEMENT_H
