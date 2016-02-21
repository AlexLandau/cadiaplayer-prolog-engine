/*
    Copyright (C) 2011 Stephan Schiffel <stephan.schiffel@gmx.de>
*/
:- module(cadia_connector).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

:- ensure_loaded([
	"match_info",
	"game_description"]).
	
:- import
	load_rules_from_file/1,
	terminal/1,
	goal/3,
	role2index/2,
	state_update/3,
	legal_moves/3
		from game_description.

:- import
	set_current_state/1
		from match_info.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% the exported predicates are used by Cadiaplayer
% and must not fail (unless there is a bug in the game
% description)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

:- reexport
	initial_state/1
		from game_description.

:- reexport
	get_current_state/1
		from match_info.

:- reexport
	parse_gdl_term_string/2,
	convert_to_gdl_string/2
		from gdl_parser.

:- export legal_moves/2.
:- mode legal_moves(++, -).
legal_moves(RoleIndex, Moves) :-
	once(role2index(Role, RoleIndex)),
	get_current_state(S),
	(legal_moves(Role, S, Moves) -> true ; Moves=[]).

:- local variable(move_counter, 0).

update_move_counter :-
	incval(move_counter),
	getval(move_counter, N),
	(N mod 1000 =:= 0 ->
		writeln(error, "====="),
		write(error, N), writeln(error, " state expansions"),
		state_stack_size(StackSize),
		write(error, StackSize), writeln(error, " states on the stack"),
		(foreach(X, [
			times, 
			session_time, 
			global_stack_used, 
			global_stack_allocated, 
			global_stack_peak, 
			trail_stack_used, 
			trail_stack_allocated, 
			trail_stack_peak, 
			control_stack_used, 
			control_stack_allocated, 
			control_stack_peak, 
			local_stack_used, 
			local_stack_allocated, 
			local_stack_peak, 
			shared_heap_allocated, 
			shared_heap_used, 
			private_heap_allocated, 
			private_heap_used, 
			gc_number, 
			gc_collected, 
			gc_area, 
			gc_ratio, 
			gc_time, 
			dictionary_entries, 
			dict_hash_usage, 
			dict_hash_collisions, 
			dict_gc_number, 
			dict_gc_time]) do
				statistics(X, Val),
				write(error, X), write(error, ": "), writeln(error, Val)
		)
	;
		true
	).

:- export make_move/2.
:- mode make_move(++, -).
make_move(JointMove, NextState) :-
	block((
	update_move_counter,
% 	writeln(make_move(JointMove, NextState)),
	get_current_state(S),
% 	writeln(current_state(S)),
	state_update(S, JointMove, NextState),
% 	writeln(next_state(NextState)),
	set_current_state(NextState)
	), Tag, (writeln(error, error(Tag, " in ", make_move(JointMove, NextState))), fail)).

:- local variable(state_stack, []).

:- export push_current_state_on_stack/0.
push_current_state_on_stack :-
	get_current_state(State),
	getval(state_stack, StateStack),
	shelf_create(state_stack_entry(State, StateStack), NewStateStack),
	setval(state_stack, NewStateStack).

:- export pop_current_state_from_stack/0.
pop_current_state_from_stack :-
	getval(state_stack, StateStack),
	(StateStack = [] ->
		true
	;
		shelf_get(StateStack, 1, State),
		set_current_state(State),
		shelf_get(StateStack, 2, PreviousStateStack),
		setval(state_stack, PreviousStateStack)
	).

state_stack_size(N) :-
	getval(state_stack, StateStack),
	state_stack_size(StateStack, N).

state_stack_size([], 0) :- !.
state_stack_size(StateStack, N) :-
	shelf_get(StateStack, 2, PreviousStateStack),
	state_stack_size(PreviousStateStack, N1),
	N is N1+1.

:- export get_goal_value/2.
:- mode get_goal_value(++, -).
get_goal_value(RoleIndex, GoalValue) :-
	once(role2index(Role, RoleIndex)),
	get_current_state(State),
	once(goal(Role, GoalValue, State)).

:- export is_terminal/1.
:- mode is_terminal(-).
is_terminal(Result) :-
	get_current_state(State),
	(terminal(State) ->
		Result=1
	;
		Result=0
	).

:- export set_rules_from_file/1.
:- mode set_rules_from_file(++).
set_rules_from_file(FileName) :-
	load_rules_from_file(FileName),
	initial_state(State),
	set_current_state(State).

:- export set_state_as_current_state/1.
:- mode set_state_as_current_state(++).
set_state_as_current_state(FluentList) :-
	sort(FluentList, State),
	set_current_state(State).
