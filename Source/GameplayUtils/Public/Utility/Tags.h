// 🐲Furkan approves of this🐲

#pragma once
#include "GameplayTagContainer.h"

// --------------------------------------------------------------------------------------

// ----- EVENT TAGS -----
#define EVENT_TAG_INTERACT FGameplayTag::RequestGameplayTag(FName("Interaction.Event.Interact"))

#define EVENT_TAG_REGISTRATION FGameplayTag::RequestGameplayTag(FName("Selection.Event.Registration"))
#define EVENT_TAG_REGISTRATION_REGISTER FGameplayTag::RequestGameplayTag(FName("Selection.Event.Registration.Register"))
#define EVENT_TAG_REGISTRATION_UNREGISTER FGameplayTag::RequestGameplayTag(FName("Selection.Event.Registration.Unregister"))

#define EVENT_TAG_SELECTION FGameplayTag::RequestGameplayTag(FName("Selection.Event.Selection"))
#define EVENT_TAG_SELECT FGameplayTag::RequestGameplayTag(FName("Selection.Event.Selection.Select"))
#define EVENT_TAG_DESELECT FGameplayTag::RequestGameplayTag(FName("Selection.Event.Selection.Deselect"))

#define EVENT_TAG_VALIDATION FGameplayTag::RequestGameplayTag(FName("Selection.Event.Validation"))
#define EVENT_TAG_VALIDATION_LOCK FGameplayTag::RequestGameplayTag(FName("Selection.Event.Validation.Lock"))
#define EVENT_TAG_VALIDATION_CONFLICT FGameplayTag::RequestGameplayTag(FName("Selection.Event.Validation.Conflict"))

#define EVENT_TAG_PROXIMITY FGameplayTag::RequestGameplayTag(FName("Selection.Event.Proximity"))
#define EVENT_TAG_PROXIMITY_ENTER FGameplayTag::RequestGameplayTag(FName("Selection.Event.Proximity.Enter"))
#define EVENT_TAG_PROXIMITY_EXIT FGameplayTag::RequestGameplayTag(FName("Selection.Event.Proximity.Exit"))
// ----- END EVENT TAGS -----

// --------------------------------------------------------------------------------------
