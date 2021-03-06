#------------------------------------------------------------------------------
#
# Copyright (c) 2014, Intel Corporation. All rights reserved.<BR>
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php.
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
# Abstract:
#
#   FSP Debug functions
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# UINT32 *
# EFIAPI
# GetStackFramePointer (
#   VOID
#   )
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(GetStackFramePointer)
ASM_PFX(GetStackFramePointer):
    mov    %ebp, %eax
    ret


