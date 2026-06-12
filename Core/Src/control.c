/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: control.c
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

#include "control.h"
#include "rtwtypes.h"
#include "rt_nonfinite.h"
#include <math.h>

/* Block signals (default storage) */
B_control_T control_B;

/* Block states (default storage) */
DW_control_T control_DW;

/* External inputs (root inport signals with default storage) */
ExtU_control_T control_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_control_T control_Y;

/* Real-time model */
static RT_MODEL_control_T control_M_;
RT_MODEL_control_T *const control_M = &control_M_;

/* Forward declaration for local functions */
static real_T control_fast_interp(const real_T xd[12], const real_T yd[12],
  real_T x);
static real_T control_fast_interp_a(const real_T xd[26], const real_T yd[26],
  real_T x);
static real_T control_fast_interp_a2(const real_T xd[370], const real_T yd[370],
  real_T x);
static void control_RK2(real_T h, real_T v, real_T pitch, real_T u_val, const
  real_T M_b[26], const real_T C_b[26], const real_T M_off[370], const real_T
  Cdr_off[370], real_T *h_final, real_T *bf, real_T *f);
static void rate_monotonic_scheduler(void);

/*
 * Set which subrates need to run this base step (base rate always runs).
 * This function must be called prior to calling the model step function
 * in order to remember which rates need to run this base step.  The
 * buffering of events allows for overlapping preemption.
 */
void control_SetEventsForThisBaseStep(boolean_T *eventFlags)
{
  /* Task runs when its counter is zero, computed via rtmStepTask macro */
  eventFlags[1] = ((boolean_T)rtmStepTask(control_M, 1));
}

/*
 *         This function updates active task flag for each subrate
 *         and rate transition flags for tasks that exchange data.
 *         The function assumes rate-monotonic multitasking scheduler.
 *         The function must be called at model base rate so that
 *         the generated code self-manages all its subrates and rate
 *         transition flags.
 */
static void rate_monotonic_scheduler(void)
{
  /* To ensure a deterministic data transfer between two rates,
   * data is transferred at the priority of a fast task and the frequency
   * of the slow task.  The following flags indicate when the data transfer
   * happens.  That is, a rate interaction flag is set true when both rates
   * will run, and false otherwise.
   */

  /* tid 0 shares data with slower tid rate: 1 */
  control_M->Timing.RateInteraction.TID0_1 = (control_M->
    Timing.TaskCounters.TID[1] == 0);

  /* Compute which subrates run during the next base time step.  Subrates
   * are an integer multiple of the base rate counter.  Therefore, the subtask
   * counter is reset when it reaches its limit (zero means run).
   */
  (control_M->Timing.TaskCounters.TID[1])++;
  if ((control_M->Timing.TaskCounters.TID[1]) > 9) {/* Sample time: [0.01s, 0.0s] */
    control_M->Timing.TaskCounters.TID[1] = 0;
  }
}

/* Function for MATLAB Function: '<S2>/NAP Function' */
static real_T control_fast_interp(const real_T xd[12], const real_T yd[12],
  real_T x)
{
  real_T y;
  if (x <= xd[0]) {
    y = yd[0];
  } else if (x >= xd[11]) {
    y = yd[11];
  } else {
    int32_T i;
    i = 0;
    int32_T exitg1;
    do {
      exitg1 = 0;
      if (i < 11) {
        real_T tmp;
        tmp = xd[i + 1];
        if (tmp >= x) {
          y = (x - xd[i]) / (tmp - xd[i]) * (yd[i + 1] - yd[i]) + yd[i];
          exitg1 = 1;
        } else {
          i++;
        }
      } else {
        y = yd[11];
        exitg1 = 1;
      }
    } while (exitg1 == 0);
  }

  return y;
}

/* Function for MATLAB Function: '<S2>/NAP Function' */
static real_T control_fast_interp_a(const real_T xd[26], const real_T yd[26],
  real_T x)
{
  real_T y;
  if (x <= xd[0]) {
    y = yd[0];
  } else if (x >= xd[25]) {
    y = yd[25];
  } else {
    int32_T i;
    i = 0;
    int32_T exitg1;
    do {
      exitg1 = 0;
      if (i < 25) {
        real_T tmp;
        tmp = xd[i + 1];
        if (tmp >= x) {
          y = (x - xd[i]) / (tmp - xd[i]) * (yd[i + 1] - yd[i]) + yd[i];
          exitg1 = 1;
        } else {
          i++;
        }
      } else {
        y = yd[25];
        exitg1 = 1;
      }
    } while (exitg1 == 0);
  }

  return y;
}

/* Function for MATLAB Function: '<S2>/NAP Function' */
static real_T control_fast_interp_a2(const real_T xd[370], const real_T yd[370],
  real_T x)
{
  real_T y;
  if (x <= xd[0]) {
    y = yd[0];
  } else if (x >= xd[369]) {
    y = yd[369];
  } else {
    int32_T i;
    i = 0;
    int32_T exitg1;
    do {
      exitg1 = 0;
      if (i < 369) {
        real_T tmp;
        tmp = xd[i + 1];
        if (tmp >= x) {
          y = (x - xd[i]) / (tmp - xd[i]) * (yd[i + 1] - yd[i]) + yd[i];
          exitg1 = 1;
        } else {
          i++;
        }
      } else {
        y = yd[369];
        exitg1 = 1;
      }
    } while (exitg1 == 0);
  }

  return y;
}

/* Function for MATLAB Function: '<S2>/NAP Function' */
static void control_RK2(real_T h, real_T v, real_T pitch, real_T u_val, const
  real_T M_b[26], const real_T C_b[26], const real_T M_off[370], const real_T
  Cdr_off[370], real_T *h_final, real_T *bf, real_T *f)
{
  real_T tmp[12];
  real_T tmp_0[12];
  real_T tmp_1[12];
  real_T cos_pitch;
  real_T mach0;
  real_T stream0;
  real_T stream1;
  real_T stream2;
  real_T v1;
  int32_T i;
  int32_T iter_rk;
  static const real_T b[12] = { 1.225, 1.112, 1.007, 0.909, 0.819, 0.736, 0.66,
    0.59, 0.526, 0.467, 0.414, 0.365 };

  cos_pitch = cos(pitch);
  *h_final = h;
  v1 = v;
  stream0 = v / cos_pitch;
  for (i = 0; i < 12; i++) {
    tmp[i] = 1000.0 * (real_T)i;
  }

  mach0 = stream0 / sqrt((288.15 - 0.0065 * h) * 401.8739);
  stream0 = -0.5 * control_fast_interp(tmp, b, h) * (stream0 * stream0);
  *bf = stream0 * control_fast_interp_a(M_b, C_b, mach0) * 0.002222324 / 25.142 *
    cos_pitch;
  *f = stream0 * control_fast_interp_a2(M_off, Cdr_off, mach0) *
    0.013478217882063612 / 25.142 * cos_pitch - 9.81;
  iter_rk = 0;
  if ((!(v <= 0.1)) && (!(h > 12000.0))) {
    for (i = 0; i < 12; i++) {
      stream0 = 1000.0 * (real_T)i;
      tmp_0[i] = stream0;
      tmp_1[i] = stream0;
    }
  }

  while ((iter_rk < 100) && (!(v1 <= 0.1)) && (!(*h_final > 12000.0))) {
    stream1 = v1 / cos_pitch;
    stream0 = stream1 / sqrt((288.15 - 0.0065 * *h_final) * 401.8739);
    mach0 = v1 * 0.2 + *h_final;
    stream1 = -0.5 * control_fast_interp(tmp_0, b, *h_final) * (stream1 *
      stream1);
    stream0 = ((stream1 * control_fast_interp_a2(M_off, Cdr_off, stream0) *
                0.013478217882063612 / 25.142 * cos_pitch - 9.81) + stream1 *
               control_fast_interp_a(M_b, C_b, stream0) * 0.002222324 / 25.142 *
               cos_pitch * u_val) * 0.2 + v1;
    stream2 = stream0 / cos_pitch;
    stream1 = stream2 / sqrt((288.15 - 0.0065 * mach0) * 401.8739);
    mach0 = -0.5 * control_fast_interp(tmp_1, b, mach0) * (stream2 * stream2);
    v1 += ((mach0 * control_fast_interp_a2(M_off, Cdr_off, stream1) *
            0.013478217882063612 / 25.142 * cos_pitch - 9.81) + mach0 *
           control_fast_interp_a(M_b, C_b, stream1) * 0.002222324 / 25.142 *
           cos_pitch * u_val) * 0.4;
    *h_final += stream0 * 0.4;
    iter_rk++;
  }
}

/* Model step function for TID0 */
void control_step0(void)               /* Sample time: [0.001s, 0.0s] */
{
  real_T b0;
  real_T e;
  real_T f0;
  real_T u0;

  {                                    /* Sample time: [0.001s, 0.0s] */
    rate_monotonic_scheduler();
  }

  /* RateTransition generated from: '<Root>/Delay' incorporates:
   *  Delay: '<Root>/Delay'
   */
  if (control_M->Timing.RateInteraction.TID0_1) {
    control_DW.TmpRTBAtDelayOutport1_Buffer[0] = control_DW.Delay_DSTATE[0];
    control_DW.TmpRTBAtDelayOutport1_Buffer[1] = control_DW.Delay_DSTATE[1];
  }

  /* End of RateTransition generated from: '<Root>/Delay' */

  /* Switch: '<Root>/Switch' incorporates:
   *  Inport: '<Root>/logic'
   */
  if (control_U.logic) {
    /* Outport: '<Root>/Out1' incorporates:
     *  Delay: '<Root>/Delay2'
     */
    control_Y.Out1 = control_DW.Delay2_DSTATE[0];
  } else {
    /* Outport: '<Root>/Out1' incorporates:
     *  Constant: '<Root>/Constant'
     */
    control_Y.Out1 = control_P.Constant_Value;
  }

  /* End of Switch: '<Root>/Switch' */

  /* RateTransition generated from: '<Root>/Enabled Subsystem' incorporates:
   *  Delay: '<Root>/Delay1'
   */
  if (control_M->Timing.RateInteraction.TID0_1) {
    control_DW.TmpRTBAtEnabledSubsystemInport3 = control_DW.Delay1_DSTATE[0];

    /* RateTransition generated from: '<Root>/Enabled Subsystem' incorporates:
     *  Delay: '<Root>/Delay1'
     */
    control_B.TmpRTBAtEnabledSubsystemOutport =
      control_DW.TmpRTBAtEnabledSubsystemOutport;

    /* RateTransition generated from: '<Root>/Enabled Subsystem' */
    control_B.TmpRTBAtEnabledSubsystemOutpo_o =
      control_DW.TmpRTBAtEnabledSubsystemOutpo_h;

    /* RateTransition generated from: '<Root>/Enabled Subsystem' */
    control_B.TmpRTBAtEnabledSubsystemOutpo_f =
      control_DW.TmpRTBAtEnabledSubsystemOutpo_m;
  }

  /* End of RateTransition generated from: '<Root>/Enabled Subsystem' */

  /* MATLAB Function: '<Root>/ESO Function' incorporates:
   *  Inport: '<Root>/Vz'
   *  Inport: '<Root>/h'
   */
  u0 = control_B.TmpRTBAtEnabledSubsystemOutport;
  b0 = control_B.TmpRTBAtEnabledSubsystemOutpo_o;
  f0 = control_B.TmpRTBAtEnabledSubsystemOutpo_f;
  if (rtIsNaN(control_B.TmpRTBAtEnabledSubsystemOutport)) {
    u0 = 0.0;
  }

  if (rtIsNaN(control_B.TmpRTBAtEnabledSubsystemOutpo_o)) {
    b0 = -5.0;
  } else if (control_B.TmpRTBAtEnabledSubsystemOutpo_o == 0.0) {
    b0 = -5.0;
  }

  if (rtIsNaN(control_B.TmpRTBAtEnabledSubsystemOutpo_f)) {
    f0 = -9.81;
  }

  if (!control_DW.z1_not_empty) {
    control_DW.z1 = 892.0;
    control_DW.z1_not_empty = true;
    control_DW.z2 = 0.0;
    control_DW.z3 = -9.81;
    control_DW.u_prev = 0.0;
    if (!rtIsNaN(control_U.h)) {
      control_DW.z1 = control_U.h;
    }

    if (!rtIsNaN(control_U.Vz)) {
      control_DW.z2 = control_U.Vz;
    }

    if (!rtIsNaN(f0)) {
      control_DW.z3 = f0;
    }
  }

  if (rtIsNaN(control_U.h) || rtIsNaN(control_U.Vz)) {
  } else {
    e = control_DW.z1 - control_U.h;
    control_DW.u_prev = fmax(0.0, fmin(1.0, fmax(fmin((fmax(-0.5, fmin(0.5,
      -control_DW.z3 / b0)) + u0) - control_DW.u_prev, 0.3), -0.3) +
      control_DW.u_prev));
    control_DW.z1 += (control_DW.z2 - 18.0 * e) * 0.001;
    control_DW.z2 += (((control_DW.z3 + f0) - 108.0 * e) + b0 *
                      control_DW.u_prev) * 0.001;
    control_DW.z3 += -216.0 * e * 0.001;
  }

  /* Outputs for Enabled SubSystem: '<Root>/Enabled Subsystem' incorporates:
   *  EnablePort: '<S2>/Enable'
   */
  /* Inport: '<Root>/logic' */
  if (control_U.logic) {
    if (!control_DW.EnabledSubsystem_MODE) {
      control_DW.EnabledSubsystem_MODE = true;
    }
  } else if (control_DW.EnabledSubsystem_MODE) {
    control_DW.EnabledSubsystem_MODE = false;
  }

  /* End of Outputs for SubSystem: '<Root>/Enabled Subsystem' */

  /* Update for Delay: '<Root>/Delay' */
  control_DW.Delay_DSTATE[0] = control_DW.Delay_DSTATE[2];
  control_DW.Delay_DSTATE[1] = control_DW.Delay_DSTATE[3];
  control_DW.Delay_DSTATE[2] = control_DW.z1;
  control_DW.Delay_DSTATE[3] = control_DW.z2;

  /* Update for Delay: '<Root>/Delay2' incorporates:
   *  MATLAB Function: '<Root>/ESO Function'
   */
  control_DW.Delay2_DSTATE[0] = control_DW.Delay2_DSTATE[1];
  control_DW.Delay2_DSTATE[1] = control_DW.u_prev;

  /* Update for Delay: '<Root>/Delay1' incorporates:
   *  Inport: '<Root>/pitch'
   */
  control_DW.Delay1_DSTATE[0] = control_DW.Delay1_DSTATE[1];
  control_DW.Delay1_DSTATE[1] = control_U.pitch;
}

/* Model step function for TID1 */
void control_step1(void)               /* Sample time: [0.01s, 0.0s] */
{
  real_T rtb_bf;
  real_T rtb_f_nominal;
  real_T rtb_u_out;
  real_T u_mid;
  boolean_T guard1;

  /* Outputs for Enabled SubSystem: '<Root>/Enabled Subsystem' incorporates:
   *  EnablePort: '<S2>/Enable'
   */
  if (control_DW.EnabledSubsystem_MODE) {
    /* Delay: '<S2>/Delay' */
    control_B.Delay = control_DW.Delay_DSTATE_h[0];

    /* Delay: '<S2>/Delay1' */
    control_B.Delay1 = control_DW.Delay1_DSTATE_h[0];

    /* Delay: '<S2>/Delay2' */
    control_B.Delay2 = control_DW.Delay2_DSTATE_i[0];

    /* MATLAB Function: '<S2>/NAP Function' incorporates:
     *  Constant: '<S2>/Constant'
     *  Constant: '<S2>/Constant1'
     *  Constant: '<S2>/Constant2'
     *  Constant: '<S2>/Constant3'
     *  RateTransition generated from: '<Root>/Delay'
     *  RateTransition generated from: '<Root>/Enabled Subsystem'
     */
    rtb_u_out = 0.0;
    rtb_bf = -4.5;
    rtb_f_nominal = -9.81;
    if ((!rtIsNaN(control_DW.TmpRTBAtDelayOutport1_Buffer[0])) && (!rtIsNaN
         (control_DW.TmpRTBAtDelayOutport1_Buffer[1]))) {
      guard1 = false;
      if (control_DW.tick <= 0.0) {
        control_RK2(control_DW.TmpRTBAtDelayOutport1_Buffer[0],
                    control_DW.TmpRTBAtDelayOutport1_Buffer[1],
                    control_DW.TmpRTBAtEnabledSubsystemInport3, 0.0,
                    control_P.M_b, control_P.C_b, control_P.M_off,
                    control_P.Cdr_off, &u_mid, &rtb_bf, &rtb_f_nominal);
        if (u_mid < 11072.32) {
        } else {
          control_RK2(control_DW.TmpRTBAtDelayOutport1_Buffer[0],
                      control_DW.TmpRTBAtDelayOutport1_Buffer[1],
                      control_DW.TmpRTBAtEnabledSubsystemInport3, 1.0,
                      control_P.M_b, control_P.C_b, control_P.M_off,
                      control_P.Cdr_off, &rtb_u_out, &rtb_bf, &rtb_f_nominal);
          if (rtb_u_out > 11072.32) {
            rtb_u_out = 1.0;
          } else {
            rtb_bf = 0.0;
            rtb_f_nominal = 1.0;
            control_RK2(control_DW.TmpRTBAtDelayOutport1_Buffer[0],
                        control_DW.TmpRTBAtDelayOutport1_Buffer[1],
                        control_DW.TmpRTBAtEnabledSubsystemInport3, 0.5,
                        control_P.M_b, control_P.C_b, control_P.M_off,
                        control_P.Cdr_off, &rtb_u_out, &control_DW.bf_store,
                        &control_DW.f_store);
            if (rtb_u_out > 11072.32) {
              rtb_bf = 0.5;
            } else {
              rtb_f_nominal = 0.5;
            }

            u_mid = (rtb_bf + rtb_f_nominal) / 2.0;
            control_RK2(control_DW.TmpRTBAtDelayOutport1_Buffer[0],
                        control_DW.TmpRTBAtDelayOutport1_Buffer[1],
                        control_DW.TmpRTBAtEnabledSubsystemInport3, u_mid,
                        control_P.M_b, control_P.C_b, control_P.M_off,
                        control_P.Cdr_off, &rtb_u_out, &control_DW.bf_store,
                        &control_DW.f_store);
            if (rtb_u_out > 11072.32) {
              rtb_bf = u_mid;
            } else {
              rtb_f_nominal = u_mid;
            }

            u_mid = (rtb_bf + rtb_f_nominal) / 2.0;
            control_RK2(control_DW.TmpRTBAtDelayOutport1_Buffer[0],
                        control_DW.TmpRTBAtDelayOutport1_Buffer[1],
                        control_DW.TmpRTBAtEnabledSubsystemInport3, u_mid,
                        control_P.M_b, control_P.C_b, control_P.M_off,
                        control_P.Cdr_off, &rtb_u_out, &control_DW.bf_store,
                        &control_DW.f_store);
            if (rtb_u_out > 11072.32) {
              rtb_bf = u_mid;
            } else {
              rtb_f_nominal = u_mid;
            }

            control_DW.u_store = (rtb_bf + rtb_f_nominal) / 2.0;
            control_DW.tick = 10.0;
            guard1 = true;
          }
        }
      } else {
        guard1 = true;
      }

      if (guard1) {
        control_DW.tick--;
        rtb_u_out = control_DW.u_store;
        rtb_bf = control_DW.bf_store;
        rtb_f_nominal = control_DW.f_store;
      }
    }

    /* End of MATLAB Function: '<S2>/NAP Function' */

    /* Update for Delay: '<S2>/Delay' */
    control_DW.Delay_DSTATE_h[0] = control_DW.Delay_DSTATE_h[1];
    control_DW.Delay_DSTATE_h[1] = rtb_u_out;

    /* Update for Delay: '<S2>/Delay1' */
    control_DW.Delay1_DSTATE_h[0] = control_DW.Delay1_DSTATE_h[1];
    control_DW.Delay1_DSTATE_h[1] = rtb_bf;

    /* Update for Delay: '<S2>/Delay2' */
    control_DW.Delay2_DSTATE_i[0] = control_DW.Delay2_DSTATE_i[1];
    control_DW.Delay2_DSTATE_i[1] = rtb_f_nominal;
  }

  /* End of Outputs for SubSystem: '<Root>/Enabled Subsystem' */

  /* RateTransition generated from: '<Root>/Enabled Subsystem' */
  control_DW.TmpRTBAtEnabledSubsystemOutport = control_B.Delay;

  /* RateTransition generated from: '<Root>/Enabled Subsystem' */
  control_DW.TmpRTBAtEnabledSubsystemOutpo_h = control_B.Delay1;

  /* RateTransition generated from: '<Root>/Enabled Subsystem' */
  control_DW.TmpRTBAtEnabledSubsystemOutpo_m = control_B.Delay2;
}

/* Model initialize function */
void control_initialize(void)
{
  /* Start for RateTransition generated from: '<Root>/Enabled Subsystem' */
  control_B.TmpRTBAtEnabledSubsystemOutport =
    control_P.TmpRTBAtEnabledSubsystemOutport;

  /* Start for RateTransition generated from: '<Root>/Enabled Subsystem' */
  control_B.TmpRTBAtEnabledSubsystemOutpo_o =
    control_P.TmpRTBAtEnabledSubsystemOutpo_j;

  /* Start for RateTransition generated from: '<Root>/Enabled Subsystem' */
  control_B.TmpRTBAtEnabledSubsystemOutpo_f =
    control_P.TmpRTBAtEnabledSubsystemOutp_jx;

  /* InitializeConditions for Delay: '<Root>/Delay' */
  control_DW.Delay_DSTATE[0] = control_P.Delay_InitialCondition_g;
  control_DW.Delay_DSTATE[1] = control_P.Delay_InitialCondition_g;
  control_DW.Delay_DSTATE[2] = control_P.Delay_InitialCondition_g;
  control_DW.Delay_DSTATE[3] = control_P.Delay_InitialCondition_g;

  /* InitializeConditions for Delay: '<Root>/Delay2' */
  control_DW.Delay2_DSTATE[0] = control_P.Delay2_InitialCondition_c;

  /* InitializeConditions for Delay: '<Root>/Delay1' */
  control_DW.Delay1_DSTATE[0] = control_P.Delay1_InitialCondition_j;

  /* InitializeConditions for Delay: '<Root>/Delay2' */
  control_DW.Delay2_DSTATE[1] = control_P.Delay2_InitialCondition_c;

  /* InitializeConditions for Delay: '<Root>/Delay1' */
  control_DW.Delay1_DSTATE[1] = control_P.Delay1_InitialCondition_j;

  /* InitializeConditions for RateTransition generated from: '<Root>/Enabled Subsystem' */
  control_DW.TmpRTBAtEnabledSubsystemOutport =
    control_P.TmpRTBAtEnabledSubsystemOutport;

  /* InitializeConditions for RateTransition generated from: '<Root>/Enabled Subsystem' */
  control_DW.TmpRTBAtEnabledSubsystemOutpo_h =
    control_P.TmpRTBAtEnabledSubsystemOutpo_j;

  /* InitializeConditions for RateTransition generated from: '<Root>/Enabled Subsystem' */
  control_DW.TmpRTBAtEnabledSubsystemOutpo_m =
    control_P.TmpRTBAtEnabledSubsystemOutp_jx;

  /* SystemInitialize for Enabled SubSystem: '<Root>/Enabled Subsystem' */
  /* InitializeConditions for Delay: '<S2>/Delay' */
  control_DW.Delay_DSTATE_h[0] = control_P.Delay_InitialCondition;

  /* InitializeConditions for Delay: '<S2>/Delay1' */
  control_DW.Delay1_DSTATE_h[0] = control_P.Delay1_InitialCondition;

  /* InitializeConditions for Delay: '<S2>/Delay2' */
  control_DW.Delay2_DSTATE_i[0] = control_P.Delay2_InitialCondition;

  /* InitializeConditions for Delay: '<S2>/Delay' */
  control_DW.Delay_DSTATE_h[1] = control_P.Delay_InitialCondition;

  /* InitializeConditions for Delay: '<S2>/Delay1' */
  control_DW.Delay1_DSTATE_h[1] = control_P.Delay1_InitialCondition;

  /* InitializeConditions for Delay: '<S2>/Delay2' */
  control_DW.Delay2_DSTATE_i[1] = control_P.Delay2_InitialCondition;

  /* SystemInitialize for MATLAB Function: '<S2>/NAP Function' */
  control_DW.bf_store = -4.5;
  control_DW.f_store = -9.81;

  /* SystemInitialize for Delay: '<S2>/Delay' incorporates:
   *  Outport: '<S2>/u0'
   */
  control_B.Delay = control_P.u0_Y0;

  /* SystemInitialize for Delay: '<S2>/Delay1' incorporates:
   *  Outport: '<S2>/b0'
   */
  control_B.Delay1 = control_P.b0_Y0;

  /* SystemInitialize for Delay: '<S2>/Delay2' incorporates:
   *  Outport: '<S2>/f0'
   */
  control_B.Delay2 = control_P.f0_Y0;

  /* End of SystemInitialize for SubSystem: '<Root>/Enabled Subsystem' */
}

/* Model terminate function */
void control_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
