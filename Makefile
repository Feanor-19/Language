all: front middle back

clean: clean_front clean_middle clean_back

.PHONY: middle
middle:
	@make -C .\middleend

.PHONY: run_middle
run_middle:
	@make run -C .\middleend

.PHONY: clean_middle
clean_middle:
	@make clean -C .\middleend

.PHONY: back
back:
	@make -C .\backend

.PHONY: run_back
run_back:
	@make run -C .\backend

.PHONY: clean_back
clean_back:
	@make clean -C .\backend

.PHONY: front
front:
	@make -C .\frontend

.PHONY: run_front
run_front:
	@make run -C .\frontend

.PHONY: clean_front
clean_front:
	@make clean -C .\frontend
