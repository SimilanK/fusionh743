/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: control.h
 *
 * Code generated for Simulink model 'control'.
 *
 * Model version                  : 1.21
 * Simulink Coder version         : 24.1 (R2024a) 19-Nov-2023
 * C/C++ source code generated on : Mon Jun  1 17:32:38 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef control_h_
#define control_h_
#ifndef control_COMMON_INCLUDES_
#define control_COMMON_INCLUDES_
#include "rtwtypes.h"
//#include "rtw_continuous.h"
//#include "rtw_solver.h"
#include "rt_nonfinite.h"
#include "math.h"
#endif                                 /* control_COMMON_INCLUDES_ */

#include "control_types.h"
#include <stddef.h>
#include "MW_target_hardware_resources.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

#ifndef rtmStepTask
#define rtmStepTask(rtm, idx)          ((rtm)->Timing.TaskCounters.TID[(idx)] == 0)
#endif

#ifndef rtmTaskCounter
#define rtmTaskCounter(rtm, idx)       ((rtm)->Timing.TaskCounters.TID[(idx)])
#endif

/* Block signals (default storage) */
typedef struct {
  real_T TmpRTBAtEnabledSubsystemOutport;/* '<Root>/Enabled Subsystem' */
  real_T TmpRTBAtEnabledSubsystemOutpo_o;/* '<Root>/Enabled Subsystem' */
  real_T TmpRTBAtEnabledSubsystemOutpo_f;/* '<Root>/Enabled Subsystem' */
  real_T Delay;                        /* '<S2>/Delay' */
  real_T Delay1;                       /* '<S2>/Delay1' */
  real_T Delay2;                       /* '<S2>/Delay2' */
} B_control_T;

/* Block states (default storage) for system '<Root>' */
typedef struct {
  real_T Delay_DSTATE[4];              /* '<Root>/Delay' */
  real_T Delay2_DSTATE[2];             /* '<Root>/Delay2' */
  real_T Delay1_DSTATE[2];             /* '<Root>/Delay1' */
  real_T Delay_DSTATE_h[2];            /* '<S2>/Delay' */
  real_T Delay1_DSTATE_h[2];           /* '<S2>/Delay1' */
  real_T Delay2_DSTATE_i[2];           /* '<S2>/Delay2' */
  real_T TmpRTBAtDelayOutport1_Buffer[2];/* synthesized block */
  real_T TmpRTBAtEnabledSubsystemInport3;/* synthesized block */
  real_T TmpRTBAtEnabledSubsystemOutport;/* synthesized block */
  real_T TmpRTBAtEnabledSubsystemOutpo_h;/* synthesized block */
  real_T TmpRTBAtEnabledSubsystemOutpo_m;/* synthesized block */
  real_T tick;                         /* '<S2>/NAP Function' */
  real_T u_store;                      /* '<S2>/NAP Function' */
  real_T bf_store;                     /* '<S2>/NAP Function' */
  real_T f_store;                      /* '<S2>/NAP Function' */
  real_T z1;                           /* '<Root>/ESO Function' */
  real_T z2;                           /* '<Root>/ESO Function' */
  real_T z3;                           /* '<Root>/ESO Function' */
  real_T u_prev;                       /* '<Root>/ESO Function' */
  boolean_T z1_not_empty;              /* '<Root>/ESO Function' */
  boolean_T EnabledSubsystem_MODE;     /* '<Root>/Enabled Subsystem' */
} DW_control_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T Vz;                           /* '<Root>/Vz' */
  real_T h;                            /* '<Root>/h' */
  boolean_T logic;                     /* '<Root>/logic' */
  real_T pitch;                        /* '<Root>/pitch' */
} ExtU_control_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T Out1;                         /* '<Root>/Out1' */
} ExtY_control_T;

/* Parameters (default storage) */
struct P_control_T_ {
  real_T C_b[26];                      /* Variable: C_b
                                        * Referenced by: '<S2>/Constant1'
                                        */
  real_T Cdr_off[370];                 /* Variable: Cdr_off
                                        * Referenced by: '<S2>/Constant3'
                                        */
  real_T M_b[26];                      /* Variable: M_b
                                        * Referenced by: '<S2>/Constant'
                                        */
  real_T M_off[370];                   /* Variable: M_off
                                        * Referenced by: '<S2>/Constant2'
                                        */
  real_T u0_Y0;                        /* Computed Parameter: u0_Y0
                                        * Referenced by: '<S2>/u0'
                                        */
  real_T b0_Y0;                        /* Computed Parameter: b0_Y0
                                        * Referenced by: '<S2>/b0'
                                        */
  real_T f0_Y0;                        /* Computed Parameter: f0_Y0
                                        * Referenced by: '<S2>/f0'
                                        */
  real_T Delay_InitialCondition;       /* Expression: 0.0
                                        * Referenced by: '<S2>/Delay'
                                        */
  real_T Delay1_InitialCondition;      /* Expression: 0.0
                                        * Referenced by: '<S2>/Delay1'
                                        */
  real_T Delay2_InitialCondition;      /* Expression: 0.0
                                        * Referenced by: '<S2>/Delay2'
                                        */
  real_T Constant_Value;               /* Expression: 0
                                        * Referenced by: '<Root>/Constant'
                                        */
  real_T Delay_InitialCondition_g;     /* Expression: 0.0
                                        * Referenced by: '<Root>/Delay'
                                        */
  real_T Delay2_InitialCondition_c;    /* Expression: 0.0
                                        * Referenced by: '<Root>/Delay2'
                                        */
  real_T Delay1_InitialCondition_j;    /* Expression: 0.0
                                        * Referenced by: '<Root>/Delay1'
                                        */
  real_T TmpRTBAtEnabledSubsystemOutport;/* Expression: 0
                                          * Referenced by:
                                          */
  real_T TmpRTBAtEnabledSubsystemOutpo_j;/* Expression: 0
                                          * Referenced by:
                                          */
  real_T TmpRTBAtEnabledSubsystemOutp_jx;/* Expression: 0
                                          * Referenced by:
                                          */
};

/* Code_Instrumentation_Declarations_Placeholder */

/* Real-time Model Data Structure */
struct tag_RTM_control_T {
  const char_T * volatile errorStatus;

  /*
   * Timing:
   * The following substructure contains information regarding
   * the timing information for the model.
   */
  struct {
    struct {
      uint8_T TID[2];
    } TaskCounters;

    struct {
      boolean_T TID0_1;
    } RateInteraction;
  } Timing;
};

/* Block parameters (default storage) */
extern P_control_T control_P;

/* Block signals (default storage) */
extern B_control_T control_B;

/* Block states (default storage) */
extern DW_control_T control_DW;

/* External inputs (root inport signals with default storage) */
extern ExtU_control_T control_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_control_T control_Y;

/* External function called from main */
extern void control_SetEventsForThisBaseStep(boolean_T *eventFlags);

/* Model entry point functions */
extern void control_initialize(void);
extern void control_step0(void);
extern void control_step1(void);
extern void control_terminate(void);

/* Real-time Model object */
extern RT_MODEL_control_T *const control_M;
extern volatile boolean_T stopRequested;
extern volatile boolean_T runModel;

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'control'
 * '<S1>'   : 'control/ESO Function'
 * '<S2>'   : 'control/Enabled Subsystem'
 * '<S3>'   : 'control/Enabled Subsystem/NAP Function'
 */
#endif                                 /* control_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
