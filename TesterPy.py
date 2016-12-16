from ..pulsar_core import TesterBase

class TesterPy(TesterBase):
    """Adds duck-typed test function and gets around no overloads."""

    def __init__(self, desc):
        super(TesterPy,self).__init__(desc)

    def test(self, desc, should_pass, expected, func, *args):
        """Same as Tester's templated test function."""
        try:
            success = (expected==func(*args) and should_pass)
        except Exception as e:
            success = not should_pass
        self.test_bool(desc,success)

    def test_value(self,desc,v1,v2,tol=0.0001):
        """Same as Tester's test with two values."""
        if type(v1)==float and type(v2)==float:
           self.test_float(desc,v1,v2,tol)
        else:
            self.test_bool(desc,v1==v2)
            

##################
# For testing on the python side
##################
def py_test_function(func, *args):
    try:
       func(*args)
    except Exception as e:
        psr.print_global_debug(str(e) + "\n")
        return 0
    except:
        return 0

    return 1



def py_test_bool_function(func, *args):
    try:
        return func(*args)

    except Exception as e:
        psr.print_global_debug(str(e) + "\n")
        return 0

    except:
        return 0    
         
