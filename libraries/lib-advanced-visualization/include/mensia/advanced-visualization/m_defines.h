/*
 * MENSIA TECHNOLOGIES CONFIDENTIAL
 * ________________________________
 *
 *  [2012] - [2013] Mensia Technologies SA
 *  Copyright, All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Mensia Technologies SA.
 * The intellectual and technical concepts contained
 * herein are proprietary to Mensia Technologies SA
 * and are covered copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Mensia Technologies SA.
 */

#ifndef __Mensia_AdvancedVisualization_Defines_H__
#define __Mensia_AdvancedVisualization_Defines_H__

//___________________________________________________________________//
//                                                                   //
// API Definition                                                    //
//___________________________________________________________________//
//                                                                   //

// Taken from
// - http://people.redhat.com/drepper/dsohowto.pdf
// - http://www.nedprod.com/programs/gccvisibility.html
#if defined LMAV_Shared
 #if defined TARGET_OS_Windows
  #define LMAV_API_Export __declspec(dllexport)
  #define LMAV_API_Import __declspec(dllimport)
 #elif defined TARGET_OS_Linux
  #define LMAV_API_Export __attribute__((visibility("default")))
  #define LMAV_API_Import __attribute__((visibility("default")))
 #else
  #define LMAV_API_Export
  #define LMAV_API_Import
 #endif
#else
 #define LMAV_API_Export
 #define LMAV_API_Import
#endif

#if defined LMAV_Exports
 #define LMAV_API LMAV_API_Export
#else
 #define LMAV_API LMAV_API_Import
#endif

#endif // __Mensia_AdvancedVisualization_Defines_H__
