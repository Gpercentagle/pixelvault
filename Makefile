all:
	@OS="$$(uname -s)"; \
	case "$$OS" in \
		Darwin*) $(MAKE) -C macos app ;; \
		Linux*)  $(MAKE) -C linux all ;; \
		*)       echo "Запустите сборку в папке windows/ через build_windows.bat" ;; \
	esac

run:
	@OS="$$(uname -s)"; \
	case "$$OS" in \
		Darwin*) $(MAKE) -C macos run ;; \
		Linux*)  $(MAKE) -C linux run ;; \
		*)       cmd //c "windows\\run.bat" ;; \
	esac

clean:
	@$(MAKE) -C macos clean 2>/dev/null || true
	@$(MAKE) -C linux clean 2>/dev/null || true

.PHONY: all run clean
