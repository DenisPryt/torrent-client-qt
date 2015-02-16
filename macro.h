#ifndef MACRO
#define MACRO

#define GETTER(varType, varName) varType Get##varName() const { return m_##varName; }

#define PROP(varType, varName) private : varType m_##varName; \
    public : GETTER(varType, varName)

#endif // MACRO
