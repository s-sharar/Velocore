"use client"

import { useState, useEffect } from "react"
import { Button } from "@/components/ui/button"
import { Input } from "@/components/ui/input"
import { Avatar, AvatarFallback, AvatarImage } from "@/components/ui/avatar"
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuTrigger,
  DropdownMenuSeparator,
} from "@/components/ui/dropdown-menu"
import { Menu, Search, Bell, Settings, User, LogOut } from "lucide-react"
import { motion } from "framer-motion"

interface TopHeaderProps {
  onMenuClick: () => void
}

interface SearchResult {
  symbol: string
  name: string
  price: number
  change: number
}

export function TopHeader({ onMenuClick }: TopHeaderProps) {
  const [searchQuery, setSearchQuery] = useState("")
  const [searchResults, setSearchResults] = useState<SearchResult[]>([])
  const [isSearching, setIsSearching] = useState(false)
  const [showResults, setShowResults] = useState(false)

  // Mock search function - replace with actual API call
  const performSearch = async (query: string) => {
    if (!query.trim()) {
      setSearchResults([])
      setShowResults(false)
      return
    }

    setIsSearching(true)

    // Simulate API call
    setTimeout(() => {
      const mockResults: SearchResult[] = [
        { symbol: "AAPL", name: "Apple Inc.", price: 175.43, change: 2.34 },
        { symbol: "GOOGL", name: "Alphabet Inc.", price: 2847.63, change: -15.23 },
        { symbol: "MSFT", name: "Microsoft Corp.", price: 338.11, change: 5.67 },
        { symbol: "TSLA", name: "Tesla Inc.", price: 248.42, change: -8.91 },
      ].filter(
        (item) =>
          item.symbol.toLowerCase().includes(query.toLowerCase()) ||
          item.name.toLowerCase().includes(query.toLowerCase()),
      )

      setSearchResults(mockResults)
      setShowResults(true)
      setIsSearching(false)
    }, 300)
  }

  useEffect(() => {
    const debounceTimer = setTimeout(() => {
      performSearch(searchQuery)
    }, 300)

    return () => clearTimeout(debounceTimer)
  }, [searchQuery])

  return (
    <header className="bg-white border-b border-gray-200 px-6 py-4">
      <div className="flex items-center justify-between">
        {/* Left side - Menu button and Logo */}
        <div className="flex items-center gap-4">
          <Button variant="ghost" size="sm" onClick={onMenuClick} className="p-2 hover:bg-gray-100">
            <Menu className="h-5 w-5 text-gray-600" />
          </Button>

          <div className="flex items-center gap-3">
            <div className="w-8 h-8 bg-gradient-to-r from-blue-600 to-purple-600 rounded-lg flex items-center justify-center">
              <span className="text-white font-bold text-sm">V</span>
            </div>
            <span className="text-xl font-semibold text-gray-900">Velocore</span>
          </div>
        </div>

        {/* Center - Search */}
        <div className="flex-1 max-w-md mx-8 relative">
          <div className="relative">
            <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 h-4 w-4 text-gray-400" />
            <Input
              type="text"
              placeholder="Search Trader, Stock, Industry, etc..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className="pl-10 pr-4 py-2 w-full bg-gray-50 border-gray-200 focus:bg-white focus:border-blue-300 focus:ring-2 focus:ring-blue-100"
            />
          </div>

          {/* Search Results Dropdown */}
          {showResults && searchResults.length > 0 && (
            <motion.div
              initial={{ opacity: 0, y: -10 }}
              animate={{ opacity: 1, y: 0 }}
              className="absolute top-full left-0 right-0 mt-2 bg-white border border-gray-200 rounded-lg shadow-lg z-50"
            >
              <div className="p-2">
                <div className="text-xs text-gray-500 px-3 py-2 border-b border-gray-100">Search Results</div>
                {searchResults.map((result) => (
                  <div
                    key={result.symbol}
                    className="flex items-center justify-between p-3 hover:bg-gray-50 rounded-lg cursor-pointer"
                    onClick={() => {
                      setSearchQuery("")
                      setShowResults(false)
                      // Handle symbol selection
                    }}
                  >
                    <div>
                      <div className="font-medium text-gray-900">{result.symbol}</div>
                      <div className="text-sm text-gray-500">{result.name}</div>
                    </div>
                    <div className="text-right">
                      <div className="font-medium text-gray-900">${result.price.toFixed(2)}</div>
                      <div className={`text-sm ${result.change >= 0 ? "text-green-600" : "text-red-600"}`}>
                        {result.change >= 0 ? "+" : ""}
                        {result.change.toFixed(2)}
                      </div>
                    </div>
                  </div>
                ))}
              </div>
            </motion.div>
          )}
        </div>

        {/* Right side - User menu */}
        <div className="flex items-center gap-3">
          <div className="text-right mr-3">
            <div className="text-sm text-gray-500">27, Oct 2022</div>
          </div>

          <Button variant="ghost" size="sm" className="p-2 hover:bg-gray-100 relative">
            <Bell className="h-5 w-5 text-gray-600" />
            <div className="absolute -top-1 -right-1 w-3 h-3 bg-red-500 rounded-full"></div>
          </Button>

          <Button variant="ghost" size="sm" className="p-2 hover:bg-gray-100">
            <Settings className="h-5 w-5 text-gray-600" />
          </Button>

          <DropdownMenu>
            <DropdownMenuTrigger asChild>
              <Button variant="ghost" className="flex items-center gap-2 p-2 hover:bg-gray-100">
                <Avatar className="h-8 w-8">
                  <AvatarImage src="/placeholder.svg?height=32&width=32" alt="User" />
                  <AvatarFallback className="bg-blue-100 text-blue-600">AD</AvatarFallback>
                </Avatar>
                <span className="text-sm font-medium text-gray-700">Anna Dust</span>
              </Button>
            </DropdownMenuTrigger>
            <DropdownMenuContent align="end" className="w-56">
              <DropdownMenuItem>
                <User className="mr-2 h-4 w-4" />
                Profile
              </DropdownMenuItem>
              <DropdownMenuItem>
                <Settings className="mr-2 h-4 w-4" />
                Settings
              </DropdownMenuItem>
              <DropdownMenuSeparator />
              <DropdownMenuItem>
                <LogOut className="mr-2 h-4 w-4" />
                Log out
              </DropdownMenuItem>
            </DropdownMenuContent>
          </DropdownMenu>
        </div>
      </div>
    </header>
  )
}
