

set(command "${make};install")
execute_process(
  COMMAND ${command}
  RESULT_VARIABLE result
  OUTPUT_FILE "/home/vkim70/src/AquetiTools/AquetiTools-prefix/src/AquetiTools-stamp/AquetiTools-install-out.log"
  ERROR_FILE "/home/vkim70/src/AquetiTools/AquetiTools-prefix/src/AquetiTools-stamp/AquetiTools-install-err.log"
  )
if(result)
  set(msg "Command failed: ${result}\n")
  foreach(arg IN LISTS command)
    set(msg "${msg} '${arg}'")
  endforeach()
  set(msg "${msg}\nSee also\n  /home/vkim70/src/AquetiTools/AquetiTools-prefix/src/AquetiTools-stamp/AquetiTools-install-*.log")
  message(FATAL_ERROR "${msg}")
else()
  set(msg "AquetiTools install command succeeded.  See also /home/vkim70/src/AquetiTools/AquetiTools-prefix/src/AquetiTools-stamp/AquetiTools-install-*.log")
  message(STATUS "${msg}")
endif()
