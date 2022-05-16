(use-modules
 (gnu packages)
 (gnu packages base)
 (gnu packages bash)
 (gnu packages fontutils)
 (gnu packages perl)
 (gnu packages xorg)
 (guix build-system gnu)
 (guix git-download)
 (guix gexp)
 (guix packages)
 (guix utils))


(define-public plan9port-luhui
  (package
   (name "plan9port")
   (version "luhui")
   (source
    (local-file
     "."
     "plan9port"
     #:recursive? #t))
   (build-system gnu-build-system)
   (arguments
    `(#:make-flags (list (string-append "CC=" ,(cc-for-target)))
      #:tests? #f                    ; no tests
      #:phases
      (modify-phases
       %standard-phases
       (delete 'configure)          ; no configure script
       (delete 'build)
       (replace
	'install            ; configure & build & install
	(lambda*
	    (#:key outputs native-inputs inputs #:allow-other-keys)
	  (let* 
	      ((out (assoc-ref outputs "out"))
	       (libxt (assoc-ref inputs "libxt"))
	       (system ,(%current-system))
	       (freetype-inc 
		(string-append (assoc-ref inputs "freetype") "/include/freetype2/"))
	       (cflags (string-append "-I" freetype-inc " ")))
	    (setenv "CFLAGS" cflags)
	    (setenv "out" out)
	    (setenv "libxt" libxt)
	    (setenv "system" system)
	    (invoke "bash" "guix/builder.sh")))))))
   (inputs
    `(("fontconfig" ,fontconfig)
      ("freetype" ,freetype)
      ("libx11" ,libx11)
      ("libxt" ,libxt)
      ("libxext" ,libxext)
      ("xorgproto" ,xorgproto)))
   (native-inputs
    `(("perl" ,perl)
      ("which" ,which)
      ("bash" ,bash)))
   (synopsis "Plan 9 from User Space")
   (home-page "https://9fans.github.io/plan9port/")
   (description "Plan 9 from User Space (aka plan9port) is a port of many Plan 9 programs from their native Plan 9 environment to Unix-like operating systems")
   (license #t)))

plan9port-luhui
