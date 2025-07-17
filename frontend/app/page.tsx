"use client"

import { useEffect, useState } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Button } from "@/components/ui/button"
import { Plus, Eye } from "lucide-react"
import { motion } from "framer-motion"
import { PriceChart } from "@/components/price-chart"
import { RecentTrades } from "@/components/recent-trades"
import { CountUp } from "@/components/count-up"
import { HotList } from "@/components/hot-list"

interface MarketData {
  symbol: string
  best_bid: number
  best_ask: number
  spread: number
  total_active_orders: number
  total_trades: number
  last_trade_stats: {
    total_trades: number
    total_volume: number
    total_value: number
    avg_price: number
    min_price: number
    max_price: number
  }
}

const containerVariants = {
  hidden: { opacity: 0 },
  visible: {
    opacity: 1,
    transition: {
      staggerChildren: 0.1,
    },
  },
}

const itemVariants = {
  hidden: { y: 20, opacity: 0 },
  visible: {
    y: 0,
    opacity: 1,
    transition: {
      type: "spring",
      stiffness: 300,
      damping: 24,
    },
  },
}

export default function Dashboard() {
  const [marketData, setMarketData] = useState<MarketData | null>(null)
  const [loading, setLoading] = useState(true)
  const [priceChange, setPriceChange] = useState<number>(0)

  useEffect(() => {
    const fetchMarketData = async () => {
      try {
        const response = await fetch("http://localhost:18080/market")
        if (response.ok) {
          const data = await response.json()
          if (marketData) {
            const oldPrice = (marketData.best_bid + marketData.best_ask) / 2
            const newPrice = (data.best_bid + data.best_ask) / 2
            setPriceChange(newPrice - oldPrice)
          }
          setMarketData(data)
        }
      } catch (error) {
        console.error("Failed to fetch market data:", error)
      } finally {
        setLoading(false)
      }
    }

    fetchMarketData()
    const interval = setInterval(fetchMarketData, 3000)
    return () => clearInterval(interval)
  }, [marketData])

  if (loading) {
    return (
      <div className="space-y-6">
        <div className="flex items-center justify-between">
          <div>
            <div className="h-8 w-32 bg-gray-200 animate-pulse rounded mb-2" />
            <div className="h-4 w-48 bg-gray-200 animate-pulse rounded" />
          </div>
        </div>
        <div className="grid gap-6 md:grid-cols-2 lg:grid-cols-4">
          {[...Array(4)].map((_, i) => (
            <Card key={i} className="border-gray-200">
              <CardHeader className="pb-2">
                <div className="h-4 w-20 bg-gray-200 animate-pulse rounded" />
              </CardHeader>
              <CardContent>
                <div className="h-8 w-24 bg-gray-200 animate-pulse rounded mb-2" />
                <div className="h-3 w-32 bg-gray-200 animate-pulse rounded" />
              </CardContent>
            </Card>
          ))}
        </div>
      </div>
    )
  }

  const midPrice = marketData ? (marketData.best_bid + marketData.best_ask) / 2 : 0

  return (
    <div className="space-y-6">
      {/* Header */}
      <motion.div initial={{ opacity: 0, y: -20 }} animate={{ opacity: 1, y: 0 }}>
        <div className="flex items-center justify-between">
          <div>
            <h1 className="text-2xl font-semibold text-gray-900">Overview</h1>
          </div>
          <div className="flex items-center gap-3">
            <Button variant="outline" size="sm" className="text-gray-600 bg-transparent">
              <Eye className="h-4 w-4 mr-2" />
              View All
            </Button>
            <Button size="sm" className="bg-blue-600 hover:bg-blue-700 text-white">
              <Plus className="h-4 w-4 mr-2" />
              New Order
            </Button>
          </div>
        </div>
      </motion.div>

      <div className="grid gap-6 lg:grid-cols-4">
        {/* Main Content - 3 columns */}
        <div className="lg:col-span-3 space-y-6">
          {/* Balance Chart */}
          <motion.div variants={itemVariants} initial="hidden" animate="visible">
            <Card className="border-gray-200">
              <CardHeader>
                <div className="flex items-center justify-between">
                  <div>
                    <CardTitle className="text-lg font-semibold text-gray-900">Balance</CardTitle>
                    <div className="flex items-center gap-4 mt-2">
                      <div className="flex items-center gap-2">
                        <div className="w-3 h-3 bg-yellow-400 rounded-full"></div>
                        <span className="text-sm text-gray-600">Income</span>
                      </div>
                      <div className="flex items-center gap-2">
                        <div className="w-3 h-3 bg-blue-500 rounded-full"></div>
                        <span className="text-sm text-gray-600">Outcome</span>
                      </div>
                    </div>
                  </div>
                  <div className="text-right">
                    <div className="text-2xl font-semibold text-blue-600">
                      $<CountUp end={1425.0} decimals={2} />
                    </div>
                    <div className="text-sm text-gray-500">28 Jul 2022</div>
                  </div>
                </div>
              </CardHeader>
              <CardContent className="h-64">
                <PriceChart />
              </CardContent>
            </Card>
          </motion.div>

          {/* Your Wallets */}
          <motion.div variants={itemVariants} initial="hidden" animate="visible">
            <Card className="border-gray-200">
              <CardHeader>
                <div className="flex items-center justify-between">
                  <CardTitle className="text-lg font-semibold text-gray-900">Your wallets</CardTitle>
                  <Button variant="ghost" size="sm">
                    <Plus className="h-5 w-5 text-gray-400" />
                  </Button>
                </div>
              </CardHeader>
              <CardContent>
                <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
                  <div className="p-4 border border-gray-200 rounded-lg text-center">
                    <div className="w-8 h-8 bg-yellow-100 rounded-full flex items-center justify-center mx-auto mb-2">
                      <span className="text-yellow-600 font-bold text-sm">₿</span>
                    </div>
                    <div className="font-medium text-gray-900">Bitcoin</div>
                    <div className="text-lg font-semibold text-gray-900 mt-1">$23,328.00</div>
                    <div className="text-xs text-gray-500">5.34923352 BTC</div>
                  </div>
                  <div className="p-4 border border-gray-200 rounded-lg text-center">
                    <div className="w-8 h-8 bg-green-100 rounded-full flex items-center justify-center mx-auto mb-2">
                      <span className="text-green-600 font-bold text-sm">₮</span>
                    </div>
                    <div className="font-medium text-gray-900">Tether</div>
                    <div className="text-lg font-semibold text-gray-900 mt-1">$69,897</div>
                    <div className="text-xs text-gray-500">69,897 USDT</div>
                  </div>
                  <div className="p-4 border border-gray-200 rounded-lg text-center">
                    <div className="w-8 h-8 bg-blue-100 rounded-full flex items-center justify-center mx-auto mb-2">
                      <span className="text-blue-600 font-bold text-sm">✕</span>
                    </div>
                    <div className="font-medium text-gray-900">Ripple</div>
                    <div className="text-lg font-semibold text-gray-900 mt-1">$206.00</div>
                    <div className="text-xs text-gray-500">0.3242422 XRP</div>
                  </div>
                  <div className="p-4 border border-gray-200 rounded-lg text-center">
                    <div className="w-8 h-8 bg-purple-100 rounded-full flex items-center justify-center mx-auto mb-2">
                      <span className="text-purple-600 font-bold text-sm">◊</span>
                    </div>
                    <div className="font-medium text-gray-900">Ethereum</div>
                    <div className="text-lg font-semibold text-gray-900 mt-1">$1,523.00</div>
                    <div className="text-xs text-gray-500">0.0000352 ETH</div>
                  </div>
                </div>
              </CardContent>
            </Card>
          </motion.div>

          {/* Transactions */}
          <motion.div variants={itemVariants} initial="hidden" animate="visible">
            <Card className="border-gray-200">
              <CardHeader>
                <div className="flex items-center justify-between">
                  <CardTitle className="text-lg font-semibold text-gray-900">Transactions</CardTitle>
                  <Button variant="ghost" size="sm" className="text-blue-600">
                    More →
                  </Button>
                </div>
              </CardHeader>
              <CardContent>
                <RecentTrades />
              </CardContent>
            </Card>
          </motion.div>
        </div>

        {/* Right Sidebar - Hot List */}
        <div className="lg:col-span-1">
          <HotList />
        </div>
      </div>
    </div>
  )
}
