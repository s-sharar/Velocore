"use client"

import { useEffect, useState } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Button } from "@/components/ui/button"
import { Eye } from "lucide-react"
import { motion } from "framer-motion"
import { PriceChart } from "@/components/price-chart"
import { RecentTrades } from "@/components/recent-trades"
import { CountUp } from "@/components/count-up"
import { MarketOverview } from "@/components/market-overview"

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
      duration: 0.3,
    },
  },
}

export default function Dashboard() {
  const [marketData, setMarketData] = useState<MarketData | null>(null)
  const [loading, setLoading] = useState(true)
  const [priceChange, setPriceChange] = useState<number>(0)
  // Dialog state is managed inside RecentTrades for now; reserved for future wiring

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

          {/* Removed mock wallets */}

          {/* Transactions */}
          <motion.div variants={itemVariants} initial="hidden" animate="visible">
          <Card className="border-gray-200">
              <CardHeader>
                <div className="flex items-center justify-between">
                  <CardTitle className="text-lg font-semibold text-gray-900">Recent Trades</CardTitle>
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

        {/* Right Sidebar - Live Market Ticks */}
        <div className="lg:col-span-1">
          <MarketOverview />
        </div>
      </div>

      {/* Trade detail dialog handled within RecentTrades */}
    </div>
  )
}
