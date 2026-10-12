{ print gensub(/"([^[:blank:]]*)"/, "[\\1]", "g", $0) }
