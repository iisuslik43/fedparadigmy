from model import *
import sys
from io import StringIO
import pytest


def get_value(number):
    old_stdout = sys.stdout
    sys.stdout = StringIO()
    scope = Scope()
    Print(number).evaluate(scope)
    value = int(sys.stdout.getvalue())
    sys.stdout = old_stdout
    return value


class TestScope:
    def test_scope_simple(self):
        parent = Scope()
        numb1 = Number(10)
        parent['a'] = numb1
        assert parent['a'] is numb1
        numb2 = Number(-2)
        parent['b'] = numb2
        assert parent['b'] is numb2

    def test_scope_take_from_parent(self):
        parent = Scope()
        scope = Scope(parent)
        numb = Number(10)
        parent['a'] = numb
        assert scope['a'] is numb

    def test_scope_have_func(self):
        parent = Scope()
        numb = Function([], [])
        parent['f'] = numb
        assert parent['f'] is numb


class TestNumber:

    def test_number(self):
        scope = Scope()
        n = Number(43)
        assert get_value(n) == 43
        assert n.evaluate(scope) is n


class TestPrint:

    def test_print_number(self):
        n = Number(43)
        assert get_value(n) == 43

    def test_print_evaluate(self, monkeypatch):
        monkeypatch.setattr(sys, 'stdout', StringIO())
        scope = Scope()
        res = Print(BinaryOperation(Number(30),
                                    '+',
                                    Number(13))).evaluate(scope)
        assert get_value(res) == 43
        assert sys.stdout.getvalue() == '43\n'


class TestRead:
    def test_read(self, monkeypatch):
        monkeypatch.setattr(sys, 'stdin', StringIO('43\n'))
        monkeypatch.setattr(sys, 'stdout', StringIO())
        scope = Scope()
        read_res = Read('a').evaluate(scope)
        Print(Reference('a')).evaluate(scope)
        assert sys.stdout.getvalue() == '43\n'
        assert get_value(read_res) == 43


class TestUnaryOperation:

    def test_unary_operation_not(self):
        scope = Scope()
        n = Number(43)
        not_n = UnaryOperation('!', n).evaluate(scope)
        assert get_value(not_n) == 0
        n = Number(0)
        not_n = UnaryOperation('!', n).evaluate(scope)
        assert get_value(not_n) != 0

    def test_unary_operation_minus(self):
        scope = Scope()
        n = Number(43)
        minus_n = UnaryOperation('-', n).evaluate(scope)
        assert get_value(minus_n) == -43

    def test_unary_operation_evaluate(self):
        scope = Scope()
        n = Number(0)
        ev = UnaryOperation('-', UnaryOperation('!', n))
        assert get_value(ev) != 0
        n = Number(43)
        ev = UnaryOperation('-', UnaryOperation('-', n))
        assert get_value(ev) == 43


class TestBinaryOperation:

    def test_binary_operation_sum(self):
        a = Number(43)
        b = Number(10)
        res = BinaryOperation(a, '+', b)
        assert get_value(res) == 53

    def test_binary_operation_dif(self):
        a = Number(43)
        b = Number(10)
        res = BinaryOperation(a, '-', b)
        assert get_value(res) == 33

    def test_binary_operation_mult(self):
        a = Number(43)
        b = Number(10)
        res = BinaryOperation(a, '*', b)
        assert get_value(res) == 430

    def test_binary_operation_del(self):
        a = Number(43)
        b = Number(10)
        res = BinaryOperation(a, '/', b)
        assert get_value(res) == 4

    def test_binary_operation_ost(self):
        scope = Scope()
        a = Number(43)
        b = Number(10)
        res = BinaryOperation(a, '%', b)
        assert get_value(res) == 3

    def test_binary_operation_and(self):
        res = BinaryOperation(Number(43), '&&', Number(0))
        assert get_value(res) == 0

    def test_binary_operation_or(self):
        res = BinaryOperation(Number(43), '||', Number(0))
        assert get_value(res) != 0

    def test_binary_operation_more(self):
        res = BinaryOperation(Number(43), '>', Number(43))
        assert get_value(res) == 0

    def test_binary_operation_less(self):
        res = BinaryOperation(Number(43), '<', Number(53))
        assert get_value(res) != 0

    def test_binary_operation_more_or_eq(self):
        res = BinaryOperation(Number(43), '>=', Number(43))
        assert get_value(res) != 0

    def test_binary_operation_less_or_eq(self):
        res = BinaryOperation(Number(43), '<=', Number(42))
        assert get_value(res) == 0

    def test_binary_operation_eq(self):
        res = BinaryOperation(Number(43), '==', Number(33))
        assert get_value(res) == 0

    def test_binary_operation_not_eq(self):
        res = BinaryOperation(Number(43), '!=', Number(33))
        assert get_value(res) != 0

    def test_binary_operation_evaluate(self):
        scope = Scope()
        a = Number(43)
        b = Number(10)
        dif = BinaryOperation(a, '-', b)
        eq = BinaryOperation(dif, '==', Number(33))
        assert get_value(eq) != 0


class TestFunction:

    def test_function_empty_arguments(self):
        scope = Scope()
        func = Function([], [Number(43)])
        res = func.evaluate(scope)
        assert get_value(res) == 43

    def test_function_many_arguments_and_evaluate(self):
        scope = Scope()
        scope['a'] = Number(33)
        scope['b'] = Number(10)
        func = Function(('a', 'b'), [BinaryOperation(Reference('a'),
                                                     '+',
                                                     Reference('b'))])
        res = func.evaluate(scope)
        assert get_value(res) == 43

    def test_function_big_body(self, monkeypatch):
        monkeypatch.setattr(sys, 'stdout', StringIO())
        scope = Scope()
        scope['a'] = Number(33)
        scope['b'] = Number(10)
        func = Function(('a', 'b'),
                        [Print(Reference('a')),
                         Reference('a'),
                         Reference('b')])
        res = func.evaluate(scope)
        Print(res).evaluate(scope)
        assert sys.stdout.getvalue() == '33\n10\n'

    def test_function_empty_body(self, monkeypatch):
        scope = Scope()
        Function(('a'), []).evaluate(scope)


class TestFunctionDefinition:

    def test_function_definition_simple(self):
        scope = Scope()
        func = Function([], [BinaryOperation(Number(10), '+', Number(33))])
        func2 = FunctionDefinition('func', func).evaluate(scope)
        assert scope['func'] is func
        assert func2 is func


class TestReference:

    def test_reference_number(self, monkeypatch):
        scope = Scope()
        a = Number(43)
        scope['a'] = a
        a2 = Reference('a').evaluate(scope)
        assert a is a2

    def test_reference_function(self, monkeypatch):
        scope = Scope()
        func = Function([], [])
        scope['function'] = func
        func2 = Reference('function').evaluate(scope)
        assert func is func2


class TestConditional:

    def test_conditional_without_if_false(self, monkeypatch):
        monkeypatch.setattr(sys, 'stdout', StringIO())
        scope = Scope()
        res1 = Conditional(Number(1), [Print(Number(1))]).evaluate(scope)
        Conditional(Number(0), [Print(Number(1))]).evaluate(scope)
        assert sys.stdout.getvalue() == '1\n'
        assert get_value(res1) == 1

    def test_conditional_evaluate_condtion(self, monkeypatch):
        monkeypatch.setattr(sys, 'stdout', StringIO())
        scope = Scope()
        Conditional(BinaryOperation(Number(3), '==', Number(3)),
                    [Print(Number(1))],
                    [Print(Number(0))]).evaluate(scope)
        Conditional(BinaryOperation(Number(2), '==', Number(3)),
                    [Print(Number(1))],
                    [Print(Number(0))]).evaluate(scope)
        assert sys.stdout.getvalue() == '1\n0\n'

    def test_conditional_return(self, monkeypatch):
        monkeypatch.setattr(sys, 'stdout', StringIO())
        scope = Scope()
        res1 = Conditional(BinaryOperation(Number(3), '==', Number(3)),
                           [Number(1)],
                           [Number(0)]).evaluate(scope)
        res2 = Conditional(BinaryOperation(Number(2), '==', Number(3)),
                           [Number(1)],
                           [Number(0)]).evaluate(scope)
        assert get_value(res1) == 1
        assert get_value(res2) == 0

    def test_conditional_empty(self, monkeypatch):
        monkeypatch.setattr(sys, 'stdout', StringIO())
        scope = Scope()
        Conditional(Number(0),
                    [Print(Number(2))],
                    []).evaluate(scope)
        Conditional(Number(43),
                    [],
                    []).evaluate(scope)
        Conditional(Number(0),
                    [],
                    []).evaluate(scope)
        Conditional(Number(43),
                    [],
                    [Print(Number(3))]).evaluate(scope)
        assert sys.stdout.getvalue() == ''

    def test_conditional_empty_with_result(self):
        scope = Scope()
        res = Conditional(Number(43),
                          [Number(34)],
                          []).evaluate(scope)
        assert get_value(res) == 34
        res = Conditional(Number(0),
                          [],
                          [Number(334)]).evaluate(scope)
        assert get_value(res) == 334

    def test_conditional_big_body(self, monkeypatch):
        monkeypatch.setattr(sys, 'stdout', StringIO())
        scope = Scope()
        Conditional(Number(43),
                    [Print(Number(1)), Print(Number(2))],
                    [Print(Number(3)), Print(Number(4))]).evaluate(scope)
        Conditional(Number(0),
                    [Print(Number(1)), Print(Number(2))],
                    [Print(Number(3)), Print(Number(4))]).evaluate(scope)
        assert sys.stdout.getvalue() == '1\n2\n3\n4\n'


class TestFunctionCall:

    def test_function_call_with_simple(self):
        parent = Scope()
        FunctionDefinition(
            'max',
            Function(('a', 'b'),
                     [Conditional(BinaryOperation(Reference('a'),
                                                  '>=',
                                                  Reference('b')),
                                  [Reference('a')],
                                  [Reference('b')])])
        ).evaluate(parent)
        n = FunctionCall(Reference('max'),
                         [Number(6), Number(5)]).evaluate(parent)
        assert get_value(n) == 6

    def test_function_call_very_tricky(self):
        parent = Scope()
        FunctionDefinition(
            'strange_max',
            Function(
                ('a', 'b'),
                [Conditional(
                    BinaryOperation(Reference('a'),
                                    '<=',
                                    Reference('b')),
                    [Reference('b')],
                    [FunctionCall(Reference('strange_max'),
                                  [Reference('b'),
                                  Reference('a')])]
                )]
            )
        ).evaluate(parent)
        n = FunctionCall(Reference('strange_max'),
                         [Number(6), Number(5)]).evaluate(parent)
        assert get_value(n) == 6


class TestIntegration:

    def test_factorial(self):
        parent = Scope()
        FunctionDefinition(
            'factorial',
            Function(
                ('a'),
                [Conditional(
                    BinaryOperation(Reference('a'),
                                    '<=',
                                    Number(1)),
                    [Reference('a')],
                    [BinaryOperation(FunctionCall(
                                        Reference('factorial'),
                                        [BinaryOperation(Reference('a'),
                                                         '-',
                                                         Number(1))]
                                     ),
                     '*', Reference('a'))]
                )]
            )
        ).evaluate(parent)
        n = FunctionCall(Reference('factorial'),
                         [Number(5)]).evaluate(parent)
        assert get_value(n) == 120

    def test_fibonacci(self):
        parent = Scope()
        FunctionDefinition(
            'fib',
            Function(
                ('a'),
                [Conditional(
                    BinaryOperation(Reference('a'),
                                    '<=',
                                    Number(1)),
                    [Reference('a')],
                    [BinaryOperation(
                        FunctionCall(
                            Reference('fib'),
                            [BinaryOperation(Reference('a'),
                                             '-',
                                             Number(1))]
                        ),
                        '+',
                        FunctionCall(Reference('fib'),
                                     [BinaryOperation(Reference('a'),
                                                      '-',
                                                      Number(2))]))]
                )]
            )
        ).evaluate(parent)
        n = FunctionCall(Reference('fib'),
                         [Number(8)]).evaluate(parent)
        assert get_value(n) == 21

    def test_average(self):
        parent = Scope()
        FunctionDefinition(
            'average',
            Function(('a', 'b'),
                     [BinaryOperation(BinaryOperation(Reference('a'),
                                                      '+',
                                                      Reference('b')),
                                      '/',
                                      Number(2))])
        ).evaluate(parent)
        n = FunctionCall(Reference('average'),
                         [Number(4), Number(6)]).evaluate(parent)
        assert get_value(n) == 5
