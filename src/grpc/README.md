# Trace Evaluator gRPC server

- Prepare LTL Property.
- Test trace if it satisfies the LTL property.


- We check the negation of the input LTL property and if it satisfies, we report a violation.
    So the input LTL property should be an intended behaviour. And we first make a negation of it.
    Then we check if the trace satisfies the negation of the LTL property. If it does, we report a violation.

