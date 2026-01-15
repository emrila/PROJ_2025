#pragma once

// Works with any enum class or integral type
namespace FlagUtils
{
	template <typename T>
	constexpr void SetFlag(T& Flags, T InFlag)
	{
		Flags |= InFlag;
	}

	template <typename T>
	constexpr void ClearFlag(T& Flags, T InFlag)
	{
		Flags &= ~InFlag;
	}

	template <typename T>
	constexpr bool HasFlag(T Flags, T InFlag)
	{
		return (Flags & InFlag) != 0;
	}

	template <typename T>
	constexpr bool HasAllFlags(T Flags, T InFlag)
	{
		return (Flags & InFlag) == InFlag;
	}
}

using FlagUtils::SetFlag;
using FlagUtils::ClearFlag;
using FlagUtils::HasFlag;
using FlagUtils::HasAllFlags;
