#!/usr/bin/ruby

class MethodDef
  def initialize(returnType, signature, optional)
    @returnType = returnType
    @signature = signature
    @optional = optional
  end

  attr_reader :returnType, :signature, :optional
  attr_writer :optional
end

class Parser
    def initialize(filename)
      @file = File.new filename, "r"
      @signatures = Hash.new
      parse
    end

    attr_reader :signatures

    private
    def addMethod(returnType, signature, optional)
      if @signatures.include? signature
        if returnType != ''
          if @signatures[signature].returnType == ''
            optional = false if @signatures[signature].optional == false
            @signatures[signature] = MethodDef.new(returnType, signature, optional)
          elsif @signatures[signature].returnType != returnType
            fail "same signature '#{signature}' but differing return types: #{returnType} and #{@signatures[signature].returnType}"
          end
        elsif not optional and @signatures[signature].optional
          @signatures[signature].optional = false
        end
      else
        @signatures[signature] = MethodDef.new(returnType, signature, optional)
      end
    end

    PARSER_RETURN_TYPE   = 0
    PARSER_RETURN_VAR    = 1
    PARSER_METHOD_NAME   = 2
    PARSER_ARGUMENT_TYPE = 3
    PARSER_ARGUMENT_VAR  = 4

    PARSER2_CLASSNAME     = 0
    PARSER2_METHODPREFIX  = 1

    PARSER3_QMETAOBJECT   = 0
    PARSER3_SCOPEDELIMIT  = 1
    PARSER3_INVOKEMETHOD  = 2
    PARSER3_OPENPARENTH   = 3
    PARSER3_BACKENDOBJ    = 4
    PARSER3_METHODNAME    = 5
    PARSER3_CONNECTION    = 6
    PARSER3_RET_OR_ARG    = 7
    PARSER3_RET_OPEN      = 8
    PARSER3_RET_TYPE      = 9
    PARSER3_RET_NAME      = 10
    PARSER3_RET_CLOSE     = 11
    PARSER3_ARG           = 12
    PARSER3_ARG_OPEN      = 13
    PARSER3_ARG_TYPE      = 14
    PARSER3_ARG_NAME      = 15
    PARSER3_ARG_CLOSE     = 16
    PARSER3_CLOSE         = 17
    PARSER3_DONE          = 18

    def parse
      inbackendcall = false
      innamedescriptioncall = false
      ininvokemethodcall = false
      optionalmethod = false
      lasttoken = ';'
      thistoken = ';'
      returnType = String.new
      signature = String.new
      parserstate = PARSER_RETURN_TYPE
      depth = 0
      tokenize do |token|
        #puts token
        lasttoken = thistoken
        thistoken = token
        token = token[1..-1] if token[0,9] == "pBACKEND_"
        if token[0,8] == "BACKEND_"
          fail if innamedescriptioncall or inbackendcall or ininvokemethodcall
          inbackendcall = true
          if token[8,3] != "GET" # skip return arg
            parserstate = PARSER_METHOD_NAME
            returnType = ''
          else
            parserstate = PARSER_RETURN_TYPE
          end
        elsif token == 'NAMEDESCRIPTIONFROMINDEX'
          fail if innamedescriptioncall or inbackendcall or ininvokemethodcall
          innamedescriptioncall = true
          parserstate = PARSER2_CLASSNAME
        elsif token == 'QMetaObject'
          fail if innamedescriptioncall or inbackendcall or ininvokemethodcall
          ininvokemethodcall = true
          parserstate = PARSER3_SCOPEDELIMIT
          optionalmethod = (lasttoken[-1,1] == '=' or lasttoken == '(') ? true : false
        elsif ininvokemethodcall
          case parserstate
          when PARSER3_BACKENDOBJ
            parserstate += 1 if token == ','
          when PARSER3_SCOPEDELIMIT
            if token == '::'
              parserstate += 1
            else
              ininvokemethodcall = false
            end
          when PARSER3_INVOKEMETHOD
            if token == 'invokeMethod'
              parserstate += 1
            else
              ininvokemethodcall = false
            end
          when PARSER3_OPENPARENTH
            fail if token != '('
            parserstate += 1
          when PARSER3_METHODNAME
            case token
            when ','
              signature += '('
              parserstate = PARSER3_CONNECTION
            else
              fail if signature.length > 0
              signature = token[1..-2]
            end
          when PARSER3_CONNECTION
            case token
            when ','
              parserstate = PARSER3_RET_OR_ARG
            when ')'
              parserstate = PARSER3_CLOSE
# the connection is optional
            when 'Q_RETURN_ARG'
              parserstate = PARSER3_RET_OPEN
            when 'Q_ARG'
              returnType = ''
              parserstate = PARSER3_ARG_OPEN
            end
          when PARSER3_RET_OR_ARG
            if token == 'Q_RETURN_ARG'
              parserstate = PARSER3_RET_OPEN
            elsif token == 'Q_ARG'
              returnType = ''
              parserstate = PARSER3_ARG_OPEN
            else
              fail "unexpected token '#{token}"
            end
          when PARSER3_RET_TYPE
            if token == ','
              parserstate += 1
            else
              if token == '*' or token == '&' or token == '<' or token == '>' or token == '::' or returnType.empty? or returnType[-1,1] == '<' or returnType[-2,2] == '::'
                returnType += token
              else
                returnType += ' ' + token
              end
            end
          when PARSER3_RET_NAME
            parserstate = PARSER3_RET_CLOSE if token == ')'
          when PARSER3_RET_CLOSE
            case token
            when ')'
              parserstate = PARSER3_CLOSE
            when ','
              parserstate = PARSER3_ARG
            end
          when PARSER3_ARG
            if token == 'Q_ARG'
              parserstate = PARSER3_ARG_OPEN
            else
              fail "unexpected token '#{token}"
            end
          when PARSER3_ARG_TYPE
            if token == ','
              parserstate += 1
            else
              signature += ' ' if signature[-1,1] =~ /\w/ and token[0,1] =~ /\w/
              signature += token
            end
          when PARSER3_ARG_NAME
            case token
            when '('
              depth += 1
            when ')'
              if depth == 0
                parserstate = PARSER3_ARG_CLOSE
              else
                depth -= 1
              end
            end
          when PARSER3_ARG_CLOSE
            case token
            when ','
              signature += ','
              parserstate = PARSER3_ARG
            when ')'
              parserstate = PARSER3_CLOSE
            else
              fail
            end
          when PARSER3_ARG_OPEN, PARSER3_RET_OPEN
            fail if token != '('
            parserstate += 1
          when PARSER3_CLOSE
            signature += ')'
            addMethod returnType, signature, optionalmethod
            ininvokemethodcall = false
            returnType = String.new
            signature = String.new
          end
        elsif innamedescriptioncall
          case parserstate
          when PARSER2_CLASSNAME
            parserstate = PARSER2_METHODPREFIX if token == ','
          when PARSER2_METHODPREFIX
            addMethod 'QSet<int>', token + 'Indexes()', false
            addMethod 'int', token + 'Name()', false
            addMethod 'int', token + 'Description()', false
            innamedescriptioncall = false
          end
        elsif inbackendcall
          next if token == '(' # skip (
          if token == ';'
            if signature.length > 0
              signature += ')'
              addMethod returnType, signature, false
              signature = String.new
              returnType = String.new
            end
            inbackendcall = false
          else
            if token == ','
              if parserstate == PARSER_ARGUMENT_VAR
                signature += ','
                parserstate = PARSER_ARGUMENT_TYPE
              else
                parserstate += 1
              end
              next
            end
            case parserstate
            when PARSER_RETURN_TYPE
              returnType += token
            when PARSER_RETURN_VAR
            when PARSER_METHOD_NAME
              next if token == ')'
              fail if token[0,1] != '"' or token[-1,1] != '"'
              fail if signature.length > 0
              signature = token[1..-2] + '('
            when PARSER_ARGUMENT_TYPE
              signature += token
            end
          end
        end
      end
    end

    def tokenize
      incomment = false
      instring = false
      laststring = ''
      @file.each_line do |line|
        line.strip!
        next if line[0..1] == "//"
        next if line[0,1] == "#" # ignore preprocessor statements
        line.split(/(\b|\s+)/).each do |token|
          if instring
            indexOfQuote = token.index '"'
            if indexOfQuote and indexOfQuote > 0
              laststring += token[0..indexOfQuote-1]
              token = token[indexOfQuote..-1]
            end
            if token[0,2] == '""'
              laststring += token[2..-1]
              next
            elsif token[0,1] == '"'
              instring = false
              yield laststring + '"'
              token = token[1..-1]
            else
              laststring += token
              next
            end
          end
          token.strip!
          next if token.empty?
          if incomment
            incomment = false if token[0..1] == "*/"
            next
          else
            if token[0..1] == "/*"
              incomment = true
              next
            end
            break if token == "//"
          end
          if token[0,1] == '"'
            laststring = token
            instring = true
            next
          end
          semicolon = token.index ';'
          if not semicolon
            if token.length > 1
              if token[-1,1] == ',' or token[-1,1] == "'"
                yield token[0..-2]
                yield token[-1,1]
              elsif token[0,1] == "'"
                yield token[0,1]
                yield token[1..-1]
              else
                yield token
              end
            else
              yield token
            end
          elsif semicolon > 0
            yield token[0..semicolon-1]
            yield ';'
            yield token[semicolon+1..-1] if token.length > semicolon + 1
          elsif (semicolon == 0 and token.length > 1)
            yield token[0,1]
            yield token[1..-1]
          else
            yield token # a single ;
          end
        end
      end
    end
end

p = Parser.new ARGV[0]
p.signatures.each do |signature,method|
  if method.optional
    puts "addMethod( \"#{method.returnType}\", \"#{signature}\", true );"
  else
    puts "addMethod( \"#{method.returnType}\", \"#{signature}\" );"
  end
end
